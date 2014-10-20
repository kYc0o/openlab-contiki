/*
 * In the build directory just under the openlab directory:
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-cn
 * -DRELEASE=2 to avoid any log_printf
 *
 */

#include <string.h>
#include "printf.h"

#include "platform.h"
#include "fiteco_lib_gwt.h"

/* Objects */
#include "constants.h"
#include "global.h"

#include "sensorCommand.h"
#include "leds_cmds.h"

#include "iotlab_gpio.h"
#include "iotlab_autotest_cn.h"

#include "control_node.h"

/* File Local variables */
#define MIN_POWER_POLL_PERIOD 1120  //in µs
static xRXFrame_t xRxFrame;
static xTXFrame_t xTxFrame;
static xSemaphoreHandle xUSART1Mutex;
static volatile uint8_t pps_count = 0;


/* global variables init */
uint32_t time0;


/* Function Protypes */
/*
 * UART interrupt Handler
 * Recognize command frame received from the A8
 * If so put an event in the high priority queue with vRX_manager as handler
 * \param arg the argument to pass to the handler function, here it is NULL
 * \param c the character received by the UART
 */
static void vCharHandler_irq(handler_arg_t arg, uint8_t c);

/*
 * Handler to manage a received command frame
 * Parse the command, execute it
 * Send a response frame
 */
static void vRX_manager();

/*
 * Parse the command frame, execute the command
 * And from these operations results, prepare the response frame
 */
static void vParseFrame();

/*
 * Initialize the response frame to be sent
 * in return to the command frame received
 */
static inline void vInitResponseFrame();


/**
 * PPS interrupt handler
 * An interrupt is received each second from the GPS PPS
 * so we increment the number of seconds elapsed from the beginning
 */
void pps_handler_irq(handler_arg_t arg) {
    (void)arg;
    pps_count++;
}

static struct {
    uint8_t channel;
    uint8_t tx_power;
} current_radio_config;


/**
 * The main function.
 * Initialize the hardware, libraries uses and variables.
 * Launch the freeRTOS scheduler
 */
int main(void) {
    /* Setup the hardware. */
    platform_init();

    /* setup UART1 handler which is connected to the gw handler */
    uart_set_rx_handler(uart_print, vCharHandler_irq, NULL);

    //create the UART1 mutex
    xUSART1Mutex = xSemaphoreCreateMutex();
    //xRadioSem = xSemaphoreCreateMutex();
    uint16_t count;
    for (count=0; count < NBR_ERROR_FRAME;count++)
    {
        xErrorFrameMutex[count] = xSemaphoreCreateMutex();
    }

    //init the sw timer
    soft_timer_init();


    //init frame especially the transition frame
    init_frame();

    //init global variable
    no_frame_error_available_defensive_issue = OK;

    //set the open node power to off and charge the battery
    fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);
    fiteco_lib_gwt_battery_charge_enable();

    //initialize the leds: green ON, red OFF
    green_led_fix();

    /* Launch the scheduler */
    platform_run();
    return 1;
}


static inline void vInitResponseFrame()
{
    // Fill known fields of response frame
    xTxFrame.sync = SYNC_BYTE;

    // set to NACK, change it later if command successful
    xTxFrame.type = xRxFrame.type;
    xTxFrame.ack = NACK;

    // default payload length is 2
    xTxFrame.len = 2;
}

static void vParseFrame()
{
    enum error_t ret;
    uint32_t conf_ina226_period_measure;
    uint16_t radio_period_measure_ms;

    vInitResponseFrame();

    /* Analyse the Command, and take corresponding action. */
    switch (xRxFrame.type) {

    case OPEN_NODE_START:
        if (DC == xRxFrame.payload[0]) {
            // DC & Charge battery
            fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
            fiteco_lib_gwt_battery_charge_enable();
        } else if (BATTERY == xRxFrame.payload[0]) {
            // Battery and no charge
            fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__BATTERY);
            fiteco_lib_gwt_battery_charge_disable();
        } else {
            // Invalid power mode
            break;
        }
        xTxFrame.ack = ACK;
        break;

    case OPEN_NODE_STOP:
        if (CHARGE == xRxFrame.payload[0])
            fiteco_lib_gwt_battery_charge_enable();
        else if (NOCHARGE == xRxFrame.payload[0])
            fiteco_lib_gwt_battery_charge_disable();
        else
            break;
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);
        xTxFrame.ack = ACK;
        break;

    case RESET_TIME:
        ret = time_ack_transition();
        if (OK != ret) {
            //send error trame
            errManager(ret);
            break;
        }
        time0 = soft_timer_time();
        xTxFrame.ack = ACK;
        break;

    case CONFIG_RADIO:
        // Reset the PHY = rf231 state machine reinitialized
        phy_reset(platform_phy);
        // Set channel and power strength
        if (phy_set_channel(platform_phy, (xRxFrame.payload[1])))
            break;
        if (phy_set_power(platform_phy, (phy_power_t) (xRxFrame.payload[0])))
            break; //NACK

        current_radio_config.channel = xRxFrame.payload[1];
        current_radio_config.tx_power = xRxFrame.payload[0];

        ret = radioConfig_ack_transition(
                xRxFrame.payload[0], xRxFrame.payload[1]);

        if (OK != ret) {
            errManager(ret);
            break;
        }
        // phy go to sleep => no as could be an on the fly configuration
        //phy_sleep(platform_phy);
        xTxFrame.ack = ACK;
        break;

    case CONFIG_RADIO_POLL:

        if (START == xRxFrame.payload[0]) {
            radio_period_measure_ms  = xRxFrame.payload[1];
            radio_period_measure_ms |= xRxFrame.payload[2] << 8;
            if (0 == radio_poll(START, radio_period_measure_ms))
                xTxFrame.ack = ACK;

        } else if (STOP == xRxFrame.payload[0]) {
            if (0 == radio_poll(STOP,  0))
                xTxFrame.ack = ACK;
        }

        break;


    case CONFIG_POWER_POLL:
        //check period of measurement in µs is ok
        //INA226_PERIOD * INA226_AVERAGE * 2
        //*2 as one ADC in the ina226 while measure bus + shunt
        conf_ina226_period_measure =
                ((tab_ina226_period[xRxFrame.payload[1] & 0x7]) *
                (tab_ina226_average[(xRxFrame.payload[1] & 0x70) >> 4]) * 2);

        //check there is something to measure on a specific power supply
        if (MEASURE_ENABLE == (xRxFrame.payload[1] & 0x80)) {

            // Check configuration
            if (!(xRxFrame.payload[0] & 0x07))
                break;
            if (!(xRxFrame.payload[0] & 0x70))
                break;
            if (MIN_POWER_POLL_PERIOD > conf_ina226_period_measure)
                break;
            //check if measurement period is not under treatment capability
            //    => assessed at ~1ms, higher in reality so some measures
            //        could be missed
            //TODO study to set correctly MIN_POWER_POLL_PERIOD

        }
        ret = set_power_poll(xRxFrame.payload[0], xRxFrame.payload[1],
                conf_ina226_period_measure);
        if (OK == ret)
            xTxFrame.ack = ACK;
        else
            errManager(ret);
        break;


    /*
     * Not implemented
     */
    case CONFIG_RADIO_NOISE:
        xTxFrame.ack = ACK;
        break;

    case CONFIG_SNIFFER:
        xTxFrame.ack = ACK;
        break;

    case CONFIG_SENSOR:
        xTxFrame.ack = ACK;
        break;


    /*
     * Leds config
     */
    case GREEN_LED_BLINK:
        green_led_blink();
        xTxFrame.ack = ACK;
        break;

    case GREEN_LED_FIX:
        green_led_fix();
        xTxFrame.ack = ACK;
        break;

    /*
     * Tests functions
     */

    case TEST_PPS:
        pps_count = 0;
        if (START == xRxFrame.payload[0])
            gpio_enable_irq(&gpio_config[3], IRQ_RISING, pps_handler_irq, NULL);
        else
            gpio_disable_irq(&gpio_config[3]);
        xTxFrame.ack = ACK;
        break;

    case TEST_GOT_PPS:
        // Return ACK if we got PPS interrupts
        if (0 != pps_count)
                xTxFrame.ack = ACK;
        break;

    case PINGPONG:
        if (START == xRxFrame.payload[0])
            cn_test_radio_pp_start(
                    current_radio_config.channel,
                    current_radio_config.tx_power);
        else
            cn_test_radio_pp_stop();
        xTxFrame.ack = ACK;
        break;

    case GPIO:
        if (START == xRxFrame.payload[0])
            cn_test_gpio_start();
        else
            cn_test_gpio_stop();
        xTxFrame.ack = ACK;
        break;

    case TST_I2C2:
        if (START == xRxFrame.payload[0])
            cn_test_i2c2_start();
        else
            cn_test_i2c2_stop();
        xTxFrame.ack = ACK;
        break;

    default:
        // send error frame
        errManager(DEFENSIVE);
        break;
    }
}

void vSendErrorFrame(uint8_t frame_nbr)
{
    vSendFrame(xErrorFrame[frame_nbr].data, xErrorFrame[frame_nbr].len + 2);
    xErrorFrame[frame_nbr].used = NO;
    xSemaphoreGive(xErrorFrameMutex[frame_nbr]);
}

//synchroneous UART transfer
void vSendFrame(uint8_t* data, int16_t len)
{
    //use mutex to ensure completion of transmission before another
    //The priority of a task that 'takes' a mutex can potentially be raised if
    //another task of higher priority attempts to obtain the same mutex.
    xSemaphoreTake(xUSART1Mutex, portMAX_DELAY);

    uart_transfer(uart_print, data, len);

    xSemaphoreGive(xUSART1Mutex);
}

static void vRX_manager(handler_arg_t arg)
{
    (void) arg;
    vParseFrame();
    vSendFrame(xTxFrame.data, xTxFrame.len + 2);
}

static void vCharHandler_irq(handler_arg_t arg, uint8_t c)
{
    static uint16_t ix = 0;
    signed portBASE_TYPE higherPriority= pdFALSE;

    if (ix == 0) {
        if (c == SYNC_BYTE)
            xRxFrame.data[ix++] = c;  // First byte, SYNC
    } else if (ix == 1) {
        if ((0 < c) && (c <= FRAME_LENGTH_MAX))
            xRxFrame.data[ix++] = c;  // Valid length byte
        else
            ix = 0;                   // Reset frame
    } else if (ix == (uint16_t) (xRxFrame.len + 1)) {
        //end of frame
        xRxFrame.data[ix] = c;
        if (EVENT_FULL == event_post_from_isr(EVENT_QUEUE_APPLI,
                    vRX_manager, NULL)) {
            errManager(APPLICATION_QUEUE_OVERFLOW);
            vInitResponseFrame();
        }
        ix = 0;
    } else {
        xRxFrame.data[ix++] = c;
    }

    if (higherPriority == pdTRUE) {
        taskYIELD();
    }
}
