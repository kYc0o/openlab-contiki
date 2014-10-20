#ifndef IOTLAB_LEDS_UTIL_H
#define IOTLAB_LEDS_UTIL_H

#include <stdint.h>
#include "soft_timer.h"

/*
 * Toggle given leds 'leds_mask' with a period of 'period' ticks
 * It will toggle every period/2 ticks
 *
 * The current state of the leds is not reinitialized
 */
void leds_blink(soft_timer_t *timer, uint32_t period, uint8_t leds_mask);


#endif //IOTLAB_LEDS_UTIL_H
