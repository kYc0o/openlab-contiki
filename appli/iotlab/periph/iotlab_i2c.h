#ifndef IOTLAB_I2C_H
#define IOTLAB_I2C_H

#include "soft_timer_delay.h"

void iotlab_i2c_init();

int iotlab_get_time(struct soft_timer_timeval *time);

int iotlab_send_event(uint16_t value);

#endif//IOTLAB_I2C_H
