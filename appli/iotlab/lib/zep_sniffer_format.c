#include <string.h>
//#include <arpa/inet.h>

#include "debug.h"
#include "zep_sniffer_format.h"

/*
 * RFC 5905                   NTPv4 Specification                 June 2010
 */
/* 1970 - 1900 in seconds */
#define JAN_1970        2208988800UL
/* 2^32 as an int */
#define FRAC       (((uint64_t)1) << 32)

#define ZEP_V2_HEADER_LEN   32
#define ZEP_V2_ACK_LEN      8

#define ZEP_V2_TYPE_DATA    1
#define ZEP_V2_TYPE_ACK     2

#define ZEP_V2_MODE_LQI     0
#define ZEP_V2_MODE_CRC     1

static const struct {
    const uint8_t preamble_1;
    const uint8_t preamble_2;
    const uint8_t zep_version;
    const uint8_t packet_type;
} zep_header = {'E', 'X', '2', ZEP_V2_TYPE_DATA};
static const uint8_t lqi_crc_mode = ZEP_V2_MODE_LQI;
static void append_data(uint8_t *dst, size_t *index, const uint8_t *src, size_t len);

/*
 * ZEP v2 Header will have the following format (if type=1/Data):
 * |Preamble|Version| Type /
 * |2 bytes |1 byte |1 byte/
 * /Channel ID|Device ID|CRC/LQI Mode|LQI Val/
 * /  1 byte  | 2 bytes |   1 byte   |1 byte /
 * /NTP Timestamp|Sequence#|Reserved|Length|
 * /   8 bytes   | 4 bytes |10 bytes|1 byte|
 */

size_t to_zep(uint8_t *dst, phy_packet_t *pkt, uint8_t channel,
        uint16_t device_id)
{

    static uint32_t packet_counter = 0;
    if (pkt->timestamp_alt.msb == 0 && pkt->timestamp_alt.lsb == 0) {
        log_error("Please set pkt->imestamp_alt values in unix time");
        return 0;
    }
    size_t index = 0;
    uint64_t timestamp_nt = 0;
    timestamp_nt |= (((uint64_t)pkt->timestamp_alt.msb + JAN_1970) << 32);
    timestamp_nt |= (((uint64_t)pkt->timestamp_alt.lsb) * FRAC) / 1000000;

    uint16_t ne_device_id   = __builtin_bswap16(device_id);
    uint32_t ne_seqno       = __builtin_bswap32(++packet_counter);
    uint32_t ne_t_msb       = __builtin_bswap32(timestamp_nt >> 32);
    uint32_t ne_t_lsb       = __builtin_bswap32(timestamp_nt | 0xFFFFFFFF);
    // TODO: use the 'eop_time_alt' values for this also
    uint16_t rx_time_len = (uint16_t) (
            (1000000 * ((uint32_t) (pkt->eop_time - pkt->timestamp))) / 32768);
    uint16_t ne_rx_time_len = __builtin_bswap16(rx_time_len);

    // Cedric added two bytes after the packet:
    // https://github.com/adjih/exp-iotlab/blob/master/tools/SnifferHelper.py
    // I think for a 'CRC' so I'm doing the same
    uint8_t length = pkt->length + 2;  // add space for a crc (fake crc in fact)
    uint8_t fake_crc[2] = {0xFF, 0xFF};

    // We use reserved_space to put some data
    //  * rx_time_len on 2 bytes in us unsigned
    //  * rssi on 1 byte signed
    uint8_t reserved[10] = {0};
    size_t idx = 0;
    append_data(reserved, &idx, (uint8_t *)&ne_rx_time_len,
            sizeof(ne_rx_time_len));
    append_data(reserved, &idx, (uint8_t *)&pkt->rssi, sizeof(uint8_t));


    // Creating the ZEP content
    append_data(dst, &index, (const uint8_t *)&zep_header,   sizeof(zep_header));

    append_data(dst, &index,            &channel,      sizeof(uint8_t));
    append_data(dst, &index, (uint8_t *)&ne_device_id, sizeof(ne_device_id));
    append_data(dst, &index,            &lqi_crc_mode, sizeof(uint8_t));
    append_data(dst, &index,            &pkt->lqi,     sizeof(uint8_t));

    append_data(dst, &index, (uint8_t *)&ne_t_msb,     sizeof(ne_t_msb));
    append_data(dst, &index, (uint8_t *)&ne_t_lsb,     sizeof(ne_t_lsb));
    append_data(dst, &index, (uint8_t *)&ne_seqno,     sizeof(ne_seqno));

    append_data(dst, &index,            reserved,      sizeof(reserved));

    // simulate 802.15.4 packet with our 'phy_packet'
    append_data(dst, &index,            &length,       sizeof(uint8_t));
    append_data(dst, &index,            pkt->raw_data, pkt->length);
    append_data(dst, &index,            fake_crc,      sizeof(fake_crc));

    return index;
}


static void append_data(uint8_t *dst, size_t *index, const uint8_t *src, size_t len)
{
    memcpy(&dst[*index], src, len);
    *index += len;
}
