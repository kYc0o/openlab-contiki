#include <string.h>
#include "platform.h"

#include "iotlab_serial.h"
#include "iotlab_time.h"
#include "iotlab_leds.h"
#include "iotlab_leds_util.h"
#include "fiteco_lib_gwt.h"

#include "constants.h"
#include "cn_control.h"

/* for flush in reset_time */
#include "cn_consumption.h"
#include "cn_radio.h"

static struct {

    void (*pre_stop_cmd)();
    void (*post_start_cmd)();


} cn_control = {
    .pre_stop_cmd = NULL,
    .post_start_cmd = NULL
};



static int32_t on_start(uint8_t cmd_type, packet_t *pkt);
static int32_t on_stop(uint8_t cmd_type, packet_t *pkt);

static int32_t set_time(uint8_t cmd_type, packet_t *pkt);
static void do_set_time(handler_arg_t arg);

static soft_timer_t green_led_alarm;
static int32_t green_led_blink(uint8_t cmd_type, packet_t *pkt);
static int32_t green_led_on(uint8_t cmd_type, packet_t *pkt);


void cn_control_start()
{
    // Configure and register all handlers
    static iotlab_serial_handler_t handler_start;
    static iotlab_serial_handler_t handler_stop;

    handler_start.cmd_type = OPEN_NODE_START;
    handler_start.handler = on_start;
    iotlab_serial_register_handler(&handler_start);

    handler_stop.cmd_type = OPEN_NODE_STOP;
    handler_stop.handler = on_stop;
    iotlab_serial_register_handler(&handler_stop);


    // set_time
    static iotlab_serial_handler_t handler_set_time;
    handler_set_time.cmd_type = SET_TIME;
    handler_set_time.handler = set_time;
    iotlab_serial_register_handler(&handler_set_time);


    // green led control
    static iotlab_serial_handler_t handler_green_led_blink;
    static iotlab_serial_handler_t handler_green_led_on;

    handler_green_led_blink.cmd_type = GREEN_LED_BLINK;
    handler_green_led_blink.handler = green_led_blink;
    iotlab_serial_register_handler(&handler_green_led_blink);

    handler_green_led_on.cmd_type = GREEN_LED_ON;
    handler_green_led_on.handler = green_led_on;
    iotlab_serial_register_handler(&handler_green_led_on);
}

void cn_control_config(void (*pre_stop_cmd)(), void (*post_start_cmd)())
{
    cn_control.pre_stop_cmd = pre_stop_cmd;
    cn_control.post_start_cmd = post_start_cmd;
}

static int32_t on_start(uint8_t cmd_type, packet_t *pkt)
{
    if (1 != pkt->length)
        return 1;

    // DC <=> charge
    if (DC == *pkt->data) {
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
        fiteco_lib_gwt_battery_charge_enable();
    } else if (BATTERY == *pkt->data) {
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__BATTERY);
        fiteco_lib_gwt_battery_charge_disable();
    } else {
        //unexpected value
        return 1;
    }

    // Run a pre_stop command before stoping
    if (cn_control.post_start_cmd != NULL)
        cn_control.post_start_cmd();

    return 0;
}

static int32_t on_stop(uint8_t cmd_type, packet_t *pkt)
{
    if (1 != pkt->length)
        return 1;

    // Run a pre_stop command before stoping
    if (cn_control.pre_stop_cmd != NULL)
        cn_control.pre_stop_cmd();

    fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);

    // Charge if DC
    if (DC == *pkt->data)
        fiteco_lib_gwt_battery_charge_enable();
    else if (BATTERY == *pkt->data)
        fiteco_lib_gwt_battery_charge_disable();
    else
        return 1;

    return 0;
}

static struct {
        struct soft_timer_timeval unix_time;
        struct soft_timer_timeval t0;
} set_time_aux;


int32_t set_time(uint8_t cmd_type, packet_t *pkt)
{
    if (8 != pkt->length)
        return 1;
    /* Save time as soon as possible */
    set_time_aux.t0 = soft_timer_time_extended();
    /* copy unix time from pkt */
    memcpy(&set_time_aux.unix_time, pkt->data, pkt->length);

    /* alloc the ack frame */
    packet_t *ack_pkt = iotlab_serial_packet_alloc();
    if (!ack_pkt)
        return 1;
    ack_pkt->data[0] = SET_TIME;
    ack_pkt->length = 1;

    if (event_post(EVENT_QUEUE_APPLI, do_set_time, ack_pkt))
        return 1;

    return 0;
}

static void do_set_time(handler_arg_t arg)
{
    packet_t *ack_pkt = (packet_t *)arg;

    /*
     * Flush measures packets
     */
    flush_current_consumption_measures();
    flush_current_rssi_measures();

    // Send the update frame
    if (iotlab_serial_send_frame(ACK_FRAME, ack_pkt)) {
        packet_free(ack_pkt);
        return;
    }

    iotlab_time_set_time(&set_time_aux.t0, &set_time_aux.unix_time);
}

/*
 * green led control
 */

int32_t green_led_blink(uint8_t cmd_type, packet_t *pkt)
{
    leds_blink(&green_led_alarm, soft_timer_s_to_ticks(1), GREEN_LED);
    return 0;
}

int32_t green_led_on(uint8_t cmd_type, packet_t *pkt)
{
    leds_blink(&green_led_alarm, 0, GREEN_LED); // stop
    leds_on(GREEN_LED);

    return 0;
}

