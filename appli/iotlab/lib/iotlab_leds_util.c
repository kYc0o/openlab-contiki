#include "iotlab_leds_util.h"
#include "platform_leds.h"

void leds_blink(soft_timer_t *timer, uint32_t period, uint8_t leds_mask)
{
    uint32_t half_period = period / 2;

    // stop current activity
    soft_timer_stop(timer);

    if (0 == half_period)
        return;  // stop blink

    /* start blinking */
    soft_timer_set_handler(timer,
            (handler_t) leds_toggle,
            (handler_arg_t) (uint32_t)leds_mask);
    soft_timer_start(timer, half_period, 1);
}
