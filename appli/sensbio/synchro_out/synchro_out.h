struct gpio_conf {
        gpio_t          port;
        gpio_pin_t      pin;
};

struct gpio_conf gpio_config[3] = {
 {
        .port      = GPIO_A,
        .pin       = GPIO_PIN_7,
 },
 {
        .port      = GPIO_A,
        .pin       = GPIO_PIN_6,
 },
 {
        .port      = GPIO_B,
        .pin       = GPIO_PIN_0,
 },
  };
  

