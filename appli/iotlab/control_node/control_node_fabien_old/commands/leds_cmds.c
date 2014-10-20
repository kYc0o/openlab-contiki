#include <stdint.h>
#include <string.h>
#include "platform.h"
#include "printf.h"

#include "phy.h"
#include "constants.h"
#include "leds_cmds.h"

static void g_led_blink();
static void r_led_blink();

//for led management, timer and tempo
soft_timer_t led_alarm;
#define LED_BASE_PERIOD soft_timer_ms_to_ticks(500)


void green_led_blink()
{
    soft_timer_stop(&led_alarm);
    soft_timer_set_handler(&led_alarm, g_led_blink, NULL);
    soft_timer_start(&led_alarm, LED_BASE_PERIOD, 1);
}

static void g_led_blink()
{
    //bit 1 is RED LED, bit 2 is GREEN LED
    //LED_1 is red, LED_0 is green
    leds_off(RED_LED);
    leds_toggle(GREEN_LED);
}

void green_led_fix()
{
    soft_timer_stop(&led_alarm);
    //bit 1 is LED_0, bit 2 is LED_1
    //LED_1 is red, LED_0 is green
    leds_off(RED_LED);
    leds_on(GREEN_LED);
}

void error_led()
{
    soft_timer_stop(&led_alarm);
    soft_timer_set_handler(&led_alarm, r_led_blink, NULL);
    soft_timer_start(&led_alarm, LED_BASE_PERIOD, 1);
}

static void r_led_blink()
{
    leds_off(GREEN_LED);
    leds_toggle(RED_LED);
}
