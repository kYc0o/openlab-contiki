#include <string.h>
#include "platform.h"

#include "event.h"


#include "constants.h"
#include "iotlab_leds.h"
#include "iotlab_time.h"
#include "iotlab_serial.h"

#include "cn_logger.h"
#include "fiteco_lib_gwt.h"
#include "cn_consumption.h"


enum {
    SEC = 1000000,
};


struct consumption_config {
    int enable;

    int p;
    int v;
    int c;
    size_t measure_size;

    uint8_t pw_conf_byte;

    int power_source;
    uint8_t period;
    uint8_t average;
#if 0
    uint32_t meas_period_us;
#endif


    packet_t *pkt;
    uint32_t t_ref_s;
};

static struct consumption_config cur_config = {0};


static int32_t config_consumption_measures(uint8_t cmd_type, packet_t *pkt);
static void do_config_consumption_measures(handler_arg_t arg);

static packet_t *alloc_pw_ack_frame(uint8_t cons_config);

#if 0
static uint16_t tab_ina226_period[8] = {
    140, 204, 332, 588, 1100, 2116, 4156, 8244
};
static uint16_t tab_ina226_average[8] = {
    1, 4, 16, 64, 128, 256, 512, 1024
};
#endif




static void consumption_measure_handler(handler_arg_t arg,
        float v, float c, float p, uint32_t measure_time);


void cn_consumption_start()
{
    // Stop sampling
    fiteco_lib_gwt_current_monitor_stop();

    // Set configure handler
    static iotlab_serial_handler_t handler_config_consumption;
    handler_config_consumption.cmd_type = CONFIG_CONSUMPTION;
    handler_config_consumption.handler = config_consumption_measures;
    iotlab_serial_register_handler(&handler_config_consumption);
}


static int parse_consumption_config(packet_t *pkt,
        struct consumption_config *conf)
{

    /*
     * Check the config arguments
     */
    uint8_t status       = pkt->data[0];  // START/STOP
    uint8_t pw_conf      = pkt->data[1];  // P, V, C and power source
    uint8_t pw_meas_rate = pkt->data[2];  // Period/average
    int invalid_config = 0;


    /* start or stop */
    conf->enable = (START == status);

    /* get power source */
    switch (pw_conf & PW_SRC_MASK) {
    case (PW_SRC_3_3V):
        conf->power_source = FITECO_GWT_CURRENT_MONITOR__OPEN_3V;
        break;
    case (PW_SRC_5V):
        conf->power_source = FITECO_GWT_CURRENT_MONITOR__OPEN_5V;
        break;
    case (PW_SRC_BATT):
        conf->power_source = FITECO_GWT_CURRENT_MONITOR__BATTERY;
        break;
    default:
        invalid_config = 1;
        break;
    }


    // set 1 on each active measure for easier counting
    conf->p = !!(pw_conf & MEASURE_POWER);
    conf->v = !!(pw_conf & MEASURE_VOLTAGE);
    conf->c = !!(pw_conf & MEASURE_CURRENT);

    conf->measure_size  = sizeof(uint32_t);  // usecs count
    conf->measure_size += sizeof(float) * (conf->p + conf->v + conf->c);

    if (!(conf->p || conf->v || conf->c))
        invalid_config = 1;  // ERR: no measures asked


    conf->period   = (pw_meas_rate & PERIOD_MASK);
    conf->average  = (pw_meas_rate & AVERAGE_MASK) >> 4;

#if 0
    //  Real measure period == 2 * (ina226_period * ina226_average)
    //  because the hardware has to measure two values: Vshunt Vbus
    conf->meas_period_us = (2 * tab_ina226_period[conf->period] * \
            tab_ina226_average[conf->average]);


    // check if measurement period is not under treatment capability
    /*
     * => assessed at ~1ms, higher in reality so some measures
     *         could be missed
     * TODO study to set correctly MIN_POWER_POLL_PERIOD
     */
    // if (MIN_POWER_POLL_PERIOD > conf->meas_period_us)
    //     invalid_config = 1;
#endif

    /* check configuration is consistent when enabling measures */
    if (conf->enable && invalid_config)
        return 1;

    /* everything fine, alloc frame needed to send config transition */
    conf->pkt = alloc_pw_ack_frame(pw_conf);
    if (NULL == conf->pkt)
        return 1;

    return 0;
}



static int32_t config_consumption_measures(uint8_t cmd_type, packet_t *pkt)
{
    static struct consumption_config conf;

    if (3 != pkt->length)
        return 1;

    /* parse pkt arguments into conf */
    if (parse_consumption_config(pkt, &conf))
        return 1;

    /*
     * Stop INA to prevent new measures event
     * Then post an event to the queue to do the config update.
     *     When the update_consumption_config will be
     *     called all the 'old' measures will have been handled correctly
     */

    fiteco_lib_gwt_current_monitor_select(
            FITECO_GWT_CURRENT_MONITOR__OFF, consumption_measure_handler, NULL);
    if (event_post(EVENT_QUEUE_APPLI, do_config_consumption_measures, &conf))
        return 1;
    // next configuration will be processed in the event handler


    /*
     * Everything went fine, send ACK to gateway
     * A config frame ack will be sent asyncronously
     */
    return 0;
}



static void do_config_consumption_measures(handler_arg_t arg)
{

    struct consumption_config *conf = (struct consumption_config *)arg;

    // Cleanup current measures
    flush_current_consumption_measures();

    /* Send the update frame */
    if (iotlab_serial_send_frame(ACK_FRAME, conf->pkt)) {
        // ERF that's really bad, config failed
        // send ERROR NOW TODO
        leds_on(RED_LED);
        cn_logger(LOGGER_ERROR, "Invalid ack pkt for consumption");
        packet_free(conf->pkt);
        return;
    }
    /* Gateway will now be able to receive new packets */

    /* Set the new configuration */
    memcpy(&cur_config, conf, sizeof(struct consumption_config));
    cur_config.pkt = NULL;

    if (cur_config.enable) {
        ina226_configure(cur_config.period, cur_config.average);
        fiteco_lib_gwt_current_monitor_select(cur_config.power_source,
                consumption_measure_handler, NULL);
    }

}


static void consumption_measure_handler(handler_arg_t arg,
        float v, float c, float p, uint32_t measure_time_ticks)
{
    struct soft_timer_timeval timestamp;
    packet_t *pkt;

    iotlab_time_extend_relative(&timestamp, measure_time_ticks);

    if (NULL == cur_config.pkt) {
        /*
         * alloc and init a new packet
         */
        pkt = iotlab_serial_packet_alloc();
        if (NULL == pkt)
            return;  // alloc failed, drop this measure

        /*
         * init new measure packet
         */
        cur_config.pkt = pkt;
        pkt->data[0] = 0;  // empty packet
        pkt->length  = 1;  // measures number byte

        /*
         * Save time reference and write it in packet
         */
        cur_config.t_ref_s = timestamp.tv_sec;  // time reference
        iotlab_serial_append_data(pkt, &cur_config.t_ref_s, sizeof(uint32_t));
    }
    pkt = cur_config.pkt;


    /*
     * add measure
     */
    pkt->data[0]++;

    /* store the number of µs since t_ref_s */
    uint32_t usecs;
    usecs = timestamp.tv_usec;
    usecs += (timestamp.tv_sec - cur_config.t_ref_s) * SEC;
    iotlab_serial_append_data(pkt, &usecs, sizeof(uint32_t));

    /* Add power consumption values */
    if (cur_config.p)
        iotlab_serial_append_data(pkt, &p, sizeof(float));
    if (cur_config.v)
        iotlab_serial_append_data(pkt, &v, sizeof(float));
    if (cur_config.c)
        iotlab_serial_append_data(pkt, &c, sizeof(float));


    /*
     * Check is packet should be sent
     */
    int send_packet = 0;

    // Packet full
    if ((pkt->length + cur_config.measure_size) > IOTLAB_SERIAL_DATA_MAX_SIZE)
        send_packet = 1;

    // No packet has been sent for a long time: around one or two seconds
    if (usecs > (2 * SEC))
        send_packet = 1;

    if (send_packet)
        flush_current_consumption_measures();
}


/* Utils functions */


static packet_t *alloc_pw_ack_frame(uint8_t pw_config)
{
    packet_t *pkt = iotlab_serial_packet_alloc();

    if (pkt) {
        pkt->data[0] = CONFIG_CONSUMPTION;
        pkt->data[1] = pw_config;
        pkt->length = 2;
    }
    return pkt;
}

void flush_current_consumption_measures()
{
    packet_t *pkt = cur_config.pkt;

    if (NULL == pkt)
        return;
    if (iotlab_serial_send_frame(CONSUMPTION_FRAME, pkt))
        packet_free(pkt);  // send fail

    cur_config.pkt = NULL;
}
