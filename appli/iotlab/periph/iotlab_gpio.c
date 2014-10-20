#include "nvic_.h"
#include "iotlab_gpio.h"



void gpio_enable_irq(struct gpio_conf *gpio, enum irq_trigger edge,
                handler_t irq_handler, handler_arg_t arg)
{
    // config gpio pin
    gpio_set_input(gpio->port, gpio->pin);
    // link gpio to irq line
    afio_select_exti_pin(gpio->exti_line, gpio->afio_port);
    nvic_enable_interrupt_line(gpio->nvic_line);
    // set IRQ handler for this irq_line triggered at `edge`
    exti_set_handler(gpio->exti_line, irq_handler, arg);
    exti_enable_interrupt_line(gpio->exti_line, edge);
}

void gpio_disable_irq(struct gpio_conf *gpio)
{
    nvic_disable_interrupt_line(gpio->nvic_line);
    exti_disable_interrupt_line(gpio->exti_line);
}


void gpio_trigger_irq_rising(struct gpio_conf *output)
{
    gpio_pin_set(output->port, output->pin);
    gpio_pin_clear(output->port, output->pin);
}


/*
 * Platform specific gpio_config init
 */


#ifdef IOTLAB_M3
struct gpio_conf gpio_config[4] = {
    {}, // allow using gpio_config[gpio_num] directly
    {
        .port      = GPIO_A,
        .pin       = GPIO_PIN_3,
        .exti_line = EXTI_LINE_Px3,
        .afio_port = AFIO_PORT_A,
        .nvic_line = NVIC_IRQ_LINE_EXTI3,
    },
    {
        .port      = GPIO_B,
        .pin       = GPIO_PIN_9,
        .exti_line = EXTI_LINE_Px9,
        .afio_port = AFIO_PORT_B,
        .nvic_line = NVIC_IRQ_LINE_EXTI9_5,
    },
    {
        .port      = GPIO_C,
        .pin       = GPIO_PIN_11,
        .exti_line = EXTI_LINE_Px11,
        .afio_port = AFIO_PORT_C,
        .nvic_line = NVIC_IRQ_LINE_EXTI15_10,
    },
};

#elif IOTLAB_A8_M3
struct gpio_conf gpio_config[4] = {
    {}, // allow using gpio_config[gpio_num] directly
    {
        .port      = GPIO_C,
        .pin       = GPIO_PIN_8,
        .exti_line = EXTI_LINE_Px8,
        .afio_port = AFIO_PORT_C,
        .nvic_line = NVIC_IRQ_LINE_EXTI9_5,
    },
    {
        .port      = GPIO_C,
        .pin       = GPIO_PIN_0,
        .exti_line = EXTI_LINE_Px0,
        .afio_port = AFIO_PORT_C,
        .nvic_line = NVIC_IRQ_LINE_EXTI0,
    },
    {
        .port      = GPIO_A,
        .pin       = GPIO_PIN_3,
        .exti_line = EXTI_LINE_Px3,
        .afio_port = AFIO_PORT_A,
        .nvic_line = NVIC_IRQ_LINE_EXTI3,
    },
};
#elif IOTLAB_CN
struct gpio_conf gpio_config[4] = {
    {}, // allow using gpio_config[gpio_num] directly
    {
        .port      = GPIO_A,
        .pin       = GPIO_PIN_0,
        .exti_line = EXTI_LINE_Px0,
        .afio_port = AFIO_PORT_A,
        .nvic_line = NVIC_IRQ_LINE_EXTI0,
    },
    {
        .port      = GPIO_C,
        .pin       = GPIO_PIN_5,
        .exti_line = EXTI_LINE_Px5,
        .afio_port = AFIO_PORT_C,
        .nvic_line = NVIC_IRQ_LINE_EXTI9_5,
    },
    {
        .port      = GPIO_A,
        .pin       = GPIO_PIN_6,
        .exti_line = EXTI_LINE_Px6,
        .afio_port = AFIO_PORT_A,
        .nvic_line = NVIC_IRQ_LINE_EXTI9_5,
    },
};
#endif

