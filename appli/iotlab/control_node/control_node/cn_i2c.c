#include <string.h>
#include "soft_timer.h"
#include "iotlab_time.h"

#include "iotlab_i2c_slave.h"
#include "iotlab_i2c/iotlab_i2c_.h"

#include "cn_i2c.h"

#include "debug.h"

void get_timestamp(struct soft_timer_timeval *time)
{
    time->tv_sec = 0;
    time->tv_sec = 0;
    iotlab_time_extend_relative(time, soft_timer_time());
}


static void rx_time(struct iotlab_i2c_handler_arg *arg)
{
    // Put saved timestamp in buffer
    memcpy(arg->payload, &arg->timestamp, sizeof(struct soft_timer_timeval));
    log_info("Send time: %u.%06u", arg->timestamp.tv_sec,
            arg->timestamp.tv_usec);
}
static struct iotlab_i2c_handler rx_time_handler = {
    .header      = IOTLAB_I2C_RX_TIME,  // RX for open node // TX for CN
    .type        = IOTLAB_I2C_SLAVE_TX,
    .payload_len = sizeof(struct soft_timer_timeval),
    .handler     = (handler_t)rx_time,
    .next        = NULL,
};

static void tx_event(struct iotlab_i2c_handler_arg *arg)
{
    uint16_t event;
    memcpy(&event, arg->payload, sizeof(uint16_t));
    log_info("Got event %u", event);
}
static struct iotlab_i2c_handler tx_event_handler = {
    .header      = IOTLAB_I2C_TX_EVENT,  // RX for open node // TX for CN
    .type        = IOTLAB_I2C_SLAVE_RX,
    .payload_len = sizeof(uint16_t),
    .handler     = (handler_t)tx_event,
    .next        = NULL,
};



void cn_i2c_start()
{
    // register handlers
    iotlab_i2c_slave_register_handler(&rx_time_handler);
    iotlab_i2c_slave_register_handler(&tx_event_handler);

    iotlab_i2c_slave_register_timestamp_fct(get_timestamp);
    iotlab_i2c_slave_start();
}

void cn_i2c_stop()
{
    iotlab_i2c_slave_stop();
}
