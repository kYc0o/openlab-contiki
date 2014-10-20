#ifndef IOTLAB_AUTOTEST_CN_H
#define IOTLAB_AUTOTEST_CN_H


/* ON<->CN gpio auto-test */
void cn_test_gpio_start(void);
void cn_test_gpio_stop(void);


/* ON<->CN i2c auto-test */
void cn_test_i2c2_start(void);
void cn_test_i2c2_stop(void);

/* ON<->CN radio test */
int cn_test_radio_pp_start(uint8_t channel, uint8_t tx_power);
void cn_test_radio_pp_stop(void);


#endif // IOTLAB_AUTOTEST_CN_H
