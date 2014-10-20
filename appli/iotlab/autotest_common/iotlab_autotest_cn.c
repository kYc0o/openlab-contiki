#include <string.h>

#include "autotest_config.h"
#include "iotlab_gpio.h"
#include "iotlab_autotest_cn.h"

#include "stm32f1xx.h"  // I2C_2
#include "i2c.h"
#include "i2c_slave.h"


/*
 * Control node GPIO autotest
 */

void cn_test_gpio_start()
{
    struct gpio_conf *input  = &gpio_config[GPIO_ON_TO_CN];
    struct gpio_conf *output = &gpio_config[GPIO_CN_TO_ON];

    /* echo irq on 'input' to 'output' gpio */
    gpio_set_output(output->port, output->pin);
    gpio_enable_irq(input, IRQ_RISING,
            (handler_t)gpio_trigger_irq_rising, (handler_arg_t)output);
}

void cn_test_gpio_stop()
{
    struct gpio_conf *input  = &gpio_config[GPIO_ON_TO_CN];
    gpio_disable_irq(input);
}


/*
 * Control node i2c2 autotest
 */

static void i2c_slave_handler(i2c_slave_event_t ev, uint8_t *data);

void cn_test_i2c2_start()
{
    i2c_enable(I2C_2, I2C_CLOCK_MODE_FAST);
    i2c_slave_set_address(I2C_2, I2C_SLAVE_ADDR);
    i2c_slave_configure(I2C_2, i2c_slave_handler);
}

void cn_test_i2c2_stop()
{
    i2c_disable(I2C_2);
}

static void i2c_slave_handler(i2c_slave_event_t ev, uint8_t *data)
{
    static char rx_buf[I2C2_MSG_SIZE];
    static uint8_t index;
    static bool rx_success = false;

    /* called from interrupt context */

    switch (ev) {
    case I2C_SLAVE_EV_RX_START:
        index = 0;
        break;
    case I2C_SLAVE_EV_TX_START:
        index = 0;
        break;
    case I2C_SLAVE_EV_STOP:
        break;

    case I2C_SLAVE_EV_RX_BYTE:
        if (index < I2C2_MSG_SIZE)
            rx_buf[index++] = *data;
        rx_success = (0 == strcmp(I2C2_ON_MSG, rx_buf));
        break;

    case I2C_SLAVE_EV_TX_BYTE:
        if (index >= I2C2_MSG_SIZE)
            break;

        if (rx_success)
            *data = I2C2_CN_MSG[index++];
        else
            *data = I2C2_CN_ERR_MSG[index++];
        break;

    default:  // I2C_SLAVE_EV_ERROR
        break;
    }
}

/*
 * Radio communication between ON and CN
 */

static void pp_rx_start(void *);
static void pp_rx_done(phy_status_t status);
static void pp_reply();



/* CN code */

static volatile int m3c_ping_pong_running;
static phy_packet_t rx_pkt = {
    .data = rx_pkt.raw_data, // static version of phy_preprare_packet
};

int cn_test_radio_pp_start(uint8_t channel, uint8_t tx_power)
{
    phy_idle(platform_phy);
    if (phy_set_channel(platform_phy, channel))
        return 1;
    if (phy_set_power(platform_phy, tx_power))
        return 1;
    m3c_ping_pong_running = 1;
    pp_rx_start(NULL);
    return 0;
}

void cn_test_radio_pp_stop()
{
    m3c_ping_pong_running = 0;
    phy_idle(platform_phy);  // stop current phy activity
}


static void pp_rx_start(void *unused)
{
    (void)unused;

    phy_idle(platform_phy);
    if (m3c_ping_pong_running)
        phy_rx_now(platform_phy, &rx_pkt, pp_rx_done);
}


static void pp_rx_done(phy_status_t status)
{
    if (status || strcmp((char *)rx_pkt.data, RADIO_PP_ON_MSG))
        pp_rx_start(NULL);  /* Error: return to RX mode */
    else
        pp_reply();         /* got correct message */
}

static void pp_reply()
{
    // Reply with 10ms delay
    static phy_packet_t tx_pkt = {
        .data     = tx_pkt.raw_data, // static version of phy_preprare_packet
        .raw_data =        RADIO_PP_CN_MSG,
        .length   = sizeof(RADIO_PP_CN_MSG),
    };
    uint32_t tx_time = soft_timer_time() + soft_timer_ms_to_ticks(10);

    phy_idle(platform_phy);
    phy_tx(platform_phy, tx_time, &tx_pkt, (phy_handler_t) pp_rx_start);
}
