#ifndef IOTLAB_LEDS_H
#define IOTLAB_LEDS_H

#include "platform_leds.h"

/*
 * LEDS aliases
 */
#ifdef IOTLAB_M3
enum {
    LEDS_MASK  = LED_0 | LED_1 | LED_2,
    GREEN_LED  = LED_0,
    RED_LED    = LED_1,
    ORANGE_LED = LED_2,
};
#elif IOTLAB_A8_M3
enum {
    LEDS_MASK  = LED_0 | LED_1 | LED_2,
    GREEN_LED  = LED_0,
    RED_LED    = LED_1,
    ORANGE_LED = LED_2,
};
#elif IOTLAB_CN
enum {
    LEDS_MASK  = LED_0 | LED_1,
    GREEN_LED  = LED_0,
    RED_LED    = LED_1,
    ORANGE_LED = LED_2,
};
#endif


#endif //IOTLAB_LEDS_H
