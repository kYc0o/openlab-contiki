#ifndef IOTLAB_GPIO_H
#define IOTLAB_GPIO_H

#include "stm32f1xx.h"
#include "exti.h"
#include "afio.h"


struct gpio_conf {
        gpio_t          port;
        gpio_pin_t      pin;
        exti_line_t     exti_line;
        afio_port_t     afio_port;
        nvic_irq_line_t nvic_line;
};
enum irq_trigger {
        IRQ_RISING  = EXTI_TRIGGER_RISING,
        IRQ_FALLING = EXTI_TRIGGER_FALLING,
        IRQ_BOTH    = EXTI_TRIGGER_BOTH,
};


// gpio_config[i] is struct gpio_conf for gpio number 'i'
// gpio_config[0] == NULL to simplify usage
extern struct gpio_conf gpio_config[4];


/* Enable irq for gpio line */
void gpio_enable_irq(struct gpio_conf *gpio, enum irq_trigger edge,
                handler_t irq_handler, handler_arg_t arg);

/* Disable irq for gpio line */
void gpio_disable_irq(struct gpio_conf *gpio);


/* Trigger a rising interrupt on gpio output line */
void gpio_trigger_irq_rising(struct gpio_conf *output);


#endif // IOTLAB_GPIO_H
