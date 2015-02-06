#include <platform.h>
#include <event.h>
#include <phy.h>

#include <unique_id.h>	// uid
#include <printf.h>	// snprintf
#include <stdbool.h>


#include "radio.h"

#define RADIO_CHANNEL 14

static void (*receive_handler)(char *from, char *data, int rssi, int lqi);

static phy_packet_t rx_packet;
static phy_packet_t tx_packet;
static void enter_rx(handler_arg_t tx_status);
static void rx_done(phy_status_t status);
static void tx_done(phy_status_t status);
static void do_send(char *data);
static void send_packet(phy_packet_t *packet);
static void process_rx_packet(handler_arg_t ignored);
static char local_address[8+1];

void radio_init(void (*receive_callback)(char *from, char*data, int rssi, int lqi))
{
	snprintf(local_address, sizeof(local_address),
		"%02x:%02x:%02x", uid->uid8[0], uid->uid8[1], uid->uid8[2]);
	receive_handler = receive_callback;
	enter_rx(NULL);
}

char * const radio_local_address()
{
	return local_address;
}

void radio_send(char *data)
{
	event_post(EVENT_QUEUE_APPLI, (handler_t)do_send, data);
}

static void enter_rx(handler_arg_t tx_status)
{
	phy_idle(platform_phy);
	phy_set_channel(platform_phy, RADIO_CHANNEL);
	phy_prepare_packet(&rx_packet);
	phy_rx_now(platform_phy, &rx_packet, rx_done);
}

static void rx_done(phy_status_t status)
{
	if (status == PHY_SUCCESS) {
		event_post(EVENT_QUEUE_APPLI, process_rx_packet, NULL);
	}
	else {
		event_post(EVENT_QUEUE_APPLI, enter_rx, NULL);
	}
}

static void process_rx_packet(handler_arg_t arg)
{
	rx_packet.data[rx_packet.length] = 0;
	rx_packet.data[sizeof(local_address)-1] = 0;
	char *from = (char*)rx_packet.data;
	char *data = from + sizeof(local_address);
	int rssi = rx_packet.rssi;
	int lqi = rx_packet.lqi;
	receive_handler(from, data, rssi, lqi);
	enter_rx(NULL);
}

static void do_send(char *data)
{
	phy_idle(platform_phy);
	phy_prepare_packet(&tx_packet);
	tx_packet.length = snprintf(
		(char*) tx_packet.data, PHY_MAX_TX_LENGTH,
		"%s %s", local_address, data);
	send_packet(&tx_packet);
}

static void retry_tx(phy_packet_t *packet);
static void send_packet(phy_packet_t *packet)
{
	int32_t channel_is_available;
	phy_cca(platform_phy, &channel_is_available);
	if (channel_is_available)
		phy_tx_now(platform_phy, packet, tx_done);
	else
		retry_tx(packet);
}

static int nb_retries = 0;
static void tx_done(phy_status_t status)
{
	if (status)
		retry_tx(&tx_packet);
	else
	{
		nb_retries = 0;
		event_post(EVENT_QUEUE_APPLI, enter_rx, NULL);
	}
}

static void retry_tx(phy_packet_t *packet)
{
	if (nb_retries > 10) {
		printf("radio: max tx retries reached: %d.\n", nb_retries);
		nb_retries = 0;
		tx_done(-1);
		return;
	}
	nb_retries ++;
	event_post(EVENT_QUEUE_APPLI, (handler_t)send_packet, packet);
	printf("radio: channel not available, retry #%d.\n", nb_retries);
}
