#ifndef IOTLAB_I2C_SLAVE_H
#define IOTLAB_I2C_SLAVE_H

#include "soft_timer_delay.h"
#include "handler.h"

typedef enum {
    IOTLAB_I2C_SLAVE_TX,
    IOTLAB_I2C_SLAVE_RX,
} i2c_msg_type_t;

struct iotlab_i2c_handler_arg {
    uint8_t payload[32];
    size_t len;
    struct soft_timer_timeval timestamp;
};

struct iotlab_i2c_handler {
    uint8_t header;
    i2c_msg_type_t type;
    uint8_t payload_len;
    handler_t handler;
    struct iotlab_i2c_handler *next;
};

void iotlab_i2c_slave_start();
void iotlab_i2c_slave_stop();
void iotlab_i2c_slave_register_timestamp_fct(
        void (*get_timestamp)(struct soft_timer_timeval *));
void iotlab_i2c_slave_register_handler(struct iotlab_i2c_handler *handler);


#endif // IOTLAB_I2C_SLAVE_H
