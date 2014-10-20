#include <stdint.h>

/*typedef enum
{
    INA226_PERIOD_140us = 0,
    INA226_PERIOD_204us = 1,
    INA226_PERIOD_332us = 2,
    INA226_PERIOD_588us = 3,
    INA226_PERIOD_1100us = 4,
    INA226_PERIOD_2116us = 5,
    INA226_PERIOD_4156us = 6,
    INA226_PERIOD_8244us = 7,
} ina226_sampling_period_t;


typedef enum
{
    INA226_AVERAGE_1 = 0,
    INA226_AVERAGE_4 = 1,
    INA226_AVERAGE_16 = 2,
    INA226_AVERAGE_64 = 3,
    INA226_AVERAGE_128 = 4,
    INA226_AVERAGE_256 = 5,
    INA226_AVERAGE_512 = 6,
    INA226_AVERAGE_1024 = 7,
} ina226_averaging_factor_t;*/

uint16_t tab_ina226_period[8] =
{
    140, 204, 332, 588, 1100, 2116, 4156, 8244
};

uint16_t tab_ina226_average[8] =
{
    1, 4, 16, 64, 128, 256, 512, 1024
};
