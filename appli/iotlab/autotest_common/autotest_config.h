#ifndef AT_CONFIG_H
#define AT_CONFIG_H

/*
 * Should use GPIO_2 on interrupt for Control Node.
 * If not, consumption measure won't work anymore after gpio test
 */

#define GPIO_ON_TO_CN 2
#define GPIO_CN_TO_ON 1


#define I2C_SLAVE_ADDR (0xAA)
#define I2C2_MSG_SIZE    8
#define I2C2_ON_MSG      "ON_MSG_"
#define I2C2_CN_MSG      "CN_MSG_"
#define I2C2_CN_ERR_MSG  "I2C_ERR"


#define RADIO_PP_ON_MSG  "TX_PKT_HELLO_WORLD"
#define RADIO_PP_CN_MSG  "RX_PKT_HELLO_WORLD"

#endif // AT_CONFIG_H
