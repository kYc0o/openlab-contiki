/*
 * Robot Broadcaster: radio_listener (running on a fixed node)
 *
 */

#include <platform.h>
#include <printf.h>
#include <string.h>

#include "event.h"
#include "radio.h"
#include <phy.h>

static void receive_callback(char *from, char* data, int rssi, int lqi)
{
	printf("broadcast;%s;RSSI;%d;LQI;%d;%s\n", from, rssi, lqi, data);
}

static void hardware_init()
{
	platform_init();
	event_init();
	soft_timer_init();

	phy_set_power(platform_phy, PHY_POWER_m30dBm);
	radio_init(receive_callback);
}

int main()
{
	hardware_init();
	platform_run();
	return 0;
}

