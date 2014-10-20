/*
 * In the build directory just under the openlab directory:
 * cmake .. -DRELEASE=4 -DPLATFORM=iotlab-cn
 * -DRELEASE=2 to avoid any log_printf but not enough
 * LOG_LEVEL_DISABLED 4 from debug.h
 *
 */

#include "platform.h"
#include "debug.h"
#include "soft_timer.h"

#include "iotlab_serial.h"
#include "iotlab_leds.h"

#include "fiteco_lib_gwt.h"

#include "cn_logger.h"
#include "cn_control.h"
#include "cn_consumption.h"
#include "cn_radio.h"
#include "cn_i2c.h"
#include "cn_autotest.h"

int main()
{
    // Initialize the platform
    platform_init();

    /* compatibility with HiKoB */
    if (uart_external == NULL)
        uart_external = uart_print;
    else
        uart_enable(uart_external, 500000);

    // Start the soft timer
    soft_timer_init();

    // Start the serial lib
    iotlab_serial_start(500000);

    // Start the application libs
    cn_control_start();

    cn_consumption_start();
    cn_radio_start();
    /* map i2c start stop to dc start/stop */
    cn_control_config(cn_i2c_stop, cn_i2c_start);
    cn_autotest_start();
    cn_logger_reset();

    //set the open node power to off and charge the battery
    fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);
    fiteco_lib_gwt_battery_charge_enable();

    //initialize the led, red off, green on
    leds_off(LEDS_MASK);
    leds_on(GREEN_LED);

    // Run
    platform_run();
    return 0;
}

