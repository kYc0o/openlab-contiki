#ifndef IOTLAB_I2C_COMMON_H
#define IOTLAB_I2C_COMMON_H

enum {
    IOTLAB_I2C_CN_ADDR = 0x42,
    IOTLAB_I2C_ERROR   = 0xFF,
};

enum {
    IOTLAB_I2C_RX_TIME,   // len 8
    IOTLAB_I2C_TX_EVENT,  // len 2
};

#endif // IOTLAB_I2C_COMMON_H
