/*
 * Smart tiles: radio pinger (to be worn by robot or human)
 *
 */

#include <platform.h>
#include <printf.h>
#include <string.h>

#include "event.h"
#include "radio.h"
#include <phy.h>

#define PING_PERIOD soft_timer_ms_to_ticks(500)
#define PING_MESSAGE "Robot!"
static soft_timer_t tx_timer;


static void radio_pinger(handler_arg_t arg)
{
	radio_send(PING_MESSAGE);
	printf("ping!\n");
}

static void receive_callback(char *from, char* data)
{
	printf("Pinger from=%s data=%s\n", from, data);
}

static void hardware_init()
{
	platform_init();
	event_init();
	soft_timer_init();

	if (phy_set_power(platform_phy, PHY_POWER_m30dBm))
		printf("failed to set radio power\n");
	radio_init(receive_callback);

	soft_timer_set_handler(&tx_timer, radio_pinger, NULL);
	soft_timer_start(&tx_timer, PING_PERIOD, 1);
}

int main()
{
	hardware_init();
	platform_run();
	return 0;
}

