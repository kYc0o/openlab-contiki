/*
 * Ping Pong game using 2 agile-fox rackets
 *
 */

#include <platform.h>
#include <printf.h>
#include <string.h>
#include <stdbool.h>

#include "blinker.h"
#include "accelero.h"
#include "radio.h"


static void button_pressed();
static void accelero_event_handler(uint32_t);
static void radio_event_handler(char *from, char *data);

static void watchdog_start();
static void watchdog_stop();
static bool is_sending_ball;
static bool is_getting_ball;

static void app_init()
{
	blinker_set_period_ms(500);
	button_set_handler(button_pressed, NULL);
	accelero_init(accelero_event_handler);
	radio_init(radio_event_handler);
	button_pressed();
	printf("started player at %s\n", radio_local_address());
}

static void grab_ball()
{
	is_sending_ball = false;
	is_getting_ball = false;
	blinker_start();
	radio_send("grab-ball");
}

static void button_pressed()
{
	grab_ball();
}

static void accelero_event_handler(uint32_t accel)
{
	if (! blinker_is_blinking() || is_sending_ball)
		return;

	is_sending_ball = true;
	radio_send("the-ball");
	watchdog_start();
}

static void on_msg_the_ball()
{
	is_getting_ball = true;
	radio_send("got-ball");
	watchdog_start();
}

static void on_msg_got_ball()
{
	if (! is_sending_ball)
		return;

	blinker_stop();
	is_sending_ball = false;
	watchdog_stop();
	radio_send("lost-ball");
}

static void on_msg_lost_ball()
{
	if (! is_getting_ball)
		return;

	watchdog_stop();
	blinker_start();
	is_getting_ball = false;
}

static void on_msg_grab_ball()
{
	is_sending_ball = false;
	blinker_stop();
}


static soft_timer_t watchdog_timer;
static void watchdog_start()
{
	soft_timer_set_handler(&watchdog_timer, (handler_t)grab_ball, NULL);
	soft_timer_start(&watchdog_timer, soft_timer_ms_to_ticks(10), 0);
}

static void watchdog_stop()
{
	soft_timer_stop(&watchdog_timer);
}

static void radio_event_handler(char *from, char *data)
{
	if (!strcmp(data, "the-ball"))
		on_msg_the_ball();
	else
	if (!strcmp(data, "got-ball"))
		on_msg_got_ball();
	else
	if (!strcmp(data, "lost-ball"))
		on_msg_lost_ball();
	else
	if (!strcmp(data, "grab-ball"))
		on_msg_grab_ball();
	else
		printf("protocol error: from=%s data=%s\n", from, data);
}

int main()
{
	platform_init();
	soft_timer_init();
	event_init();
	app_init();
	platform_run();
	return 0;
}
