#include <string.h>
#include "platform.h"
#include "i2c.h"
#include "iotlab_i2c.h"
#include "iotlab_i2c_.h"

static struct {
    i2c_t i2c;
    uint8_t addr;
} iotlab_i2c;


void iotlab_i2c_init()
{
    iotlab_i2c.i2c  = i2c1;
    iotlab_i2c.addr = IOTLAB_I2C_CN_ADDR;
}

int iotlab_get_time(struct soft_timer_timeval *time)
{
    uint8_t cmd = IOTLAB_I2C_RX_TIME;
    return i2c_tx_rx(iotlab_i2c.i2c, iotlab_i2c.addr, &cmd, 1,
            (uint8_t *)time, 8);
}

int iotlab_send_event(uint16_t value)
{
    uint8_t buf[3] = {[0] = IOTLAB_I2C_TX_EVENT};
    memcpy(&buf[1], &value, 2);
    return i2c_tx(iotlab_i2c.i2c, iotlab_i2c.addr, buf, 3);
}
