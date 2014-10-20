#include <string.h>
#include "platform.h"

#include "gps_synced_clock.h"

#define LOG_LEVEL LOG_LEVEL_INFO
#include "debug.h"

/* [CA] the following was copied and adapted from 
   openlab/appli/iotlab_tests/foren6_sniffer/foren6_sniffer.c
   Itself was: */
/*
 *
 * Copied and adapted from:
 *     https://github.com/cetic/contiki/blob/sniffer/examples/sniffer/sniffer.c
 *
 */


// Dropped all the serial configuration of the channel



  
//
//   From packet-zep.c in wireshark:
//   ZEP v2 Header will have the following format (if type=1/Data):
//   |Preamble|Version| Type |Channel ID|Device ID|CRC/LQI Mode|LQI Val|NTP Timestamp|Sequence#|Reserved|Length|
//   |2 bytes |1 byte |1 byte|  1 byte  | 2 bytes |   1 byte   |1 byte |   8 bytes   | 4 bytes |10 bytes|1 byte|
//



#define MAGIC_LEN 3
#define ZEP_VERSION 2






/*
 * RFC 5905                   NTPv4 Specification                 June 2010
 */
/* 1970 - 1900 in seconds */
#define JAN_1970        2208988800UL
/* 2^32 as an int */
#define FRAC       (((uint64_t)1) << 32)

static const uint8_t magic[] = { 'E', 'X', ZEP_VERSION };
static void sniff_rx(phy_status_t status);
static uint8_t current_channel = 0;

static void timestamp_handler(uint32_t* seconds, uint32_t* subseconds)
{
    gps_synced_time_t time_now;
    gps_synced_clock_get(&time_now);

    *seconds = time_now.s;
    *subseconds = time_now.ms*1000 + time_now.us;
    return;
}

void init_sniffer(uint8_t channel)
{
    log_info("ZEPv2 Sniffer with channel: %u", channel);
    phy_set_channel(platform_phy, channel);
    register_timestamp_handler(timestamp_handler);
    current_channel = channel;
    sniff_rx(~0);  // call without a valid status to start rx
}

static void sniff_rx(phy_status_t status)
{
    static uint32_t packet_counter = 0;
    log_debug("status: %u", status);
/* magic | type | len | pkt | crc | crc_ok | rssi | lqi | timestamp */
    static uint8_t data[256] = {
          [0] = 0xff, /* total size, filled later */
          [1] = 'E',
          [2] = 'X',
          [3] = ZEP_VERSION,
	  [4] = 1, /* type: data */
    };
   uint8_t index = 5;

    /* Two packets handling */
    static struct {
        phy_packet_t pkt_buf[2];
        int index;
    } sniff = {
        .index = 0,
        .pkt_buf = {
            // static version of phy_preprare_packet
            [0] = { .data = sniff.pkt_buf[0].raw_data },
            [1] = { .data = sniff.pkt_buf[1].raw_data },
        },
    };
    // get current packet and toggle next packet
    phy_packet_t *rx_pkt = &sniff.pkt_buf[sniff.index];
    sniff.index = (sniff.index + 1) % 2;

    log_debug("sniff.index: %u", sniff.index);

    // Enter RX again
    phy_status_t ret;
    ret = phy_rx_now(platform_phy, &sniff.pkt_buf[sniff.index], sniff_rx);
    if (ret != PHY_SUCCESS)
        log_error("PHY RX FAILED");


    /*
     * Handle received packet and dump it to serial line
     */

    if (status != PHY_SUCCESS)
        return;

    /*|Channel ID|Device ID|CRC/LQI Mode|LQI Val|NTP Timestamp|Sequence#|Reserved|Length|*/

    // Channel ID
    data[index++] = current_channel;

    // Device ID
    uint16_t device_id = 0; /* XXX: TODO */
    data[index++] = 0xFF & (device_id >> 8);
    data[index++] = 0xFF & (device_id);

    // CRC/LQI Mode
    data[index++] = 0; /* LQI mode */

    //  LQI
    // data[index++] = rx_pkt->lqi; <- XXX not used, because ...
    data[index++] = rx_pkt->rssi; /* ... rssi often more meaningful than lqi */

    // Timestamp: XXX: change to NTP format
    printf("\nTIMESTAMP_MSB : %d", rx_pkt->timestamp_alt.msb);
    printf("\nTIMESTAMP : %d\n", rx_pkt->timestamp_alt.lsb);
    uint64_t timestamp = 0;
    timestamp |= (((uint64_t)rx_pkt->timestamp_alt.msb + JAN_1970) << 32);
    timestamp |= (((uint64_t)rx_pkt->timestamp_alt.lsb) * FRAC) / 1000000;
    int i;
    for (i=0;i<8;i++)
        data[index++] = 0xFF & (timestamp >> (8*(7-i)));

    // Packet counter (32 bits)
    packet_counter++;
    data[index++] = 0xFF & (packet_counter >> (8*3));
    data[index++] = 0xFF & (packet_counter >> (8*2));
    data[index++] = 0xFF & (packet_counter >> 8);
    data[index++] = 0xFF & (packet_counter);

    // reserved
    for (i=0 ;i<10; i++)
        data[index++] = 0; 

    // len
    const uint8_t len   = rx_pkt->length;
    data[index++] = len + 2;
    log_debug("len %u", len);

    // payload
    memcpy(&data[index], rx_pkt->data, len);
    index += len;

    // trailer
    data[index++] = 0xff;
    data[index++] = 0xff;

    log_debug("final len: %u", index);

    data[0] = index-1; // fill length

    uart_transfer(uart_print, data, index);
    log_debug("uart transferted");
}
