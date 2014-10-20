/*
 * idle_with_dc
 *
 * Sets power mode to 'dc'
 * It can switch power mode off by sending '0'
 * and on again by sending '1' on serial port
 *
 *  Created on: Nov 05, 2013
 *      Author: Gaetan Harter
 */

#include "uart.h"
#include "event.h"

#include "fiteco_lib_gwt.h"
#include "platform.h"

enum
{
        GREEN_LED = 1 << 0,
        RED_LED   = 1 << 1,
};



static void char_rx_irq(handler_arg_t arg, uint8_t c);
static void char_rx_handler(handler_arg_t arg);

static void char_rx_irq(handler_arg_t arg, uint8_t c)
{
        if (c == '0' || c == '1') {
                event_post_from_isr(EVENT_QUEUE_APPLI, char_rx_handler,
                                    (handler_arg_t) ((unsigned int) c));
        }
}

static void char_rx_handler(handler_arg_t arg)
{
       char value = ((unsigned int) arg & 0xFF);

       if (value == '0') {
               fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);
               fiteco_lib_gwt_battery_charge_enable();
               leds_off(0xFF);
               leds_on(RED_LED | GREEN_LED);
       } else if (value == '1') {
               fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
               fiteco_lib_gwt_battery_charge_enable();
               leds_off(0xFF);
               leds_on(RED_LED);
       } else {
               ;
       }
}

int main()
{
        // Initialize the platform
        platform_init();

        uart_set_rx_handler(uart_print, char_rx_irq, NULL);

        event_post(EVENT_QUEUE_APPLI, char_rx_handler,
                   (handler_arg_t) ((unsigned int) '1'));

        // Run
        platform_run();
        return 0;
}
