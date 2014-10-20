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

#include "cn_i2c.h"

static void char_rx(handler_arg_t arg, uint8_t c);
static void handle_cmd(handler_arg_t arg);

static void handle_cmd(handler_arg_t arg)
{
    switch ((char) (uint32_t) arg) {
    case '1':
        printf("Start ON\n");
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
        break;
    case '0':
        printf("Stop ON\n");
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);
        break;
    case 'e':
        printf("enable i2c\n");
        cn_i2c_start();
        break;
    case 'd':
        printf("Disable i2c\n");
        cn_i2c_stop();
        break;
    case '\n':
        printf("\n");
        break;
    case '\r':
        return;
    case 'h':
    default:
        printf("\th:\tprint this help\n");
        printf("\t1:\tstart open node\n");
        printf("\t0:\tstop  open node\n");
        printf("\te:\tenable  i2c\n");
        printf("\td:\tdisable i2c\n");
        break;
    }
    printf("cmd > ");
}

int main()
{
    // Initialize the platform
    platform_init();
    //fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);

    /* compatibility with HiKoB */
    if (uart_external == NULL)
        uart_external = uart_print;
    else
        uart_enable(uart_external, 500000);


    // Start the soft timer
    soft_timer_init();


    //set the open node power to off and charge the battery
    //fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
    fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);
    fiteco_lib_gwt_battery_charge_enable();
    fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
    cn_i2c_start();

    //initialize the led, red off, green on
    leds_off(LEDS_MASK);
    leds_on(GREEN_LED);

    uart_set_rx_handler(uart_print, char_rx, NULL);

    // Run
    platform_run();
    return 0;
}



static void char_rx(handler_arg_t arg, uint8_t c) {
    event_post_from_isr(EVENT_QUEUE_APPLI, handle_cmd,
            (handler_arg_t)(uint32_t) c);
}

