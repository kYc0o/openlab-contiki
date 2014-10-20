/*
 * Ping Pong game using 2 agile-fox rackets
 *
 * Blinker widget (the ball)
 *
 */

#include <platform.h>
#include <soft_timer.h>

#include "blinker.h"

// from inline function definition in soft_timer_delay.h
#define soft_timer_ms_to_ticks_(ms) \
    ((((int64_t) ms) * SOFT_TIMER_FREQUENCY) / 1000)
#define SOFT_TIMER_PERIODIC 1
#define SOFT_TIMER_ONE_SHOT 0

static int blinker_period	= soft_timer_ms_to_ticks_(100);
static int blinker_led		= LED_0;
static void blinker_handler();
static soft_timer_t blinker_timer;

void blinker_start()
{
	if (blinker_is_blinking()) blinker_stop();

	leds_on(blinker_led);
	soft_timer_set_handler(&blinker_timer, blinker_handler, NULL);
	soft_timer_start(&blinker_timer, blinker_period, SOFT_TIMER_PERIODIC);
}

static void blinker_handler()
{
	leds_toggle(blinker_led);
}

void blinker_stop()
{
	soft_timer_stop(&blinker_timer);
	leds_off(blinker_led);
}

bool blinker_is_blinking()
{
	return soft_timer_is_active(&blinker_timer);
}

inline void blinker_toggle_blinking()
{
	blinker_is_blinking() ? blinker_stop() : blinker_start();
}

void blinker_set_period_ms(int period_ms)
{
	blinker_period = soft_timer_ms_to_ticks(period_ms);
}
