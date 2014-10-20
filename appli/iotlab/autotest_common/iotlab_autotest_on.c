#include <string.h>

#include "iotlab_gpio.h"
#include "iotlab_autotest_on.h"
#include "autotest_config.h"

#include "i2c.h"



/*
 * Open node GPIO autotest
 */
static volatile unsigned int gpio_irq_count;
static xSemaphoreHandle gpio_test_ok;

static void on_test_gpio_irq(handler_arg_t arg)
{
    signed portBASE_TYPE yield = 0;
    struct gpio_conf *output = (struct gpio_conf *)arg;

    gpio_irq_count++;

    if (5 > gpio_irq_count)
        gpio_trigger_irq_rising(output);  // continue GPIO test
    else
        xSemaphoreGiveFromISR(gpio_test_ok,  &yield);  // test successfull

    if (yield)
        portYIELD();
}


int on_test_gpio()
{
    struct gpio_conf *input  = &gpio_config[GPIO_CN_TO_ON];
    struct gpio_conf *output = &gpio_config[GPIO_ON_TO_CN];
    int ret;

    // setup GPIO modes and default values
    gpio_test_ok = xSemaphoreCreateCounting(1, 0);
    gpio_irq_count = 0;
    gpio_set_output(output->port, output->pin);
    gpio_enable_irq(input, IRQ_RISING, on_test_gpio_irq, output);

    // start test
    gpio_trigger_irq_rising(output);

    // wait finished
    ret = (pdTRUE != xSemaphoreTake(gpio_test_ok, portTICK_RATE_MS * 1000));

    gpio_disable_irq(input);
    return ret;
}


/*
 * I2C2 Test
 */
/* Send a message and expect a certain answer */
char *on_test_i2c2()
{
    uint8_t buf[I2C2_MSG_SIZE] = { [0 ... I2C2_MSG_SIZE -1] = '\0'};

    if (i2c_tx(i2c1, I2C_SLAVE_ADDR, (uint8_t*)I2C2_ON_MSG, I2C2_MSG_SIZE))
        return "I2C2_WRITE_FAIL";

    if (i2c_rx(i2c1, I2C_SLAVE_ADDR, buf, I2C2_MSG_SIZE))
        return "I2C2_READ_FAIL";
    if (strcmp(I2C2_CN_MSG, (char *)buf))
        return "I2C2_WRONG_ANSWER";

    return NULL;
}


/*
 * Radio communication between ON and CN
 */


