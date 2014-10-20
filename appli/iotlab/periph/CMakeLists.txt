include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    ${PROJECT_SOURCE_DIR}/appli/iotlab/periph
    )

add_library(iotlab_gpio      STATIC iotlab_gpio)
add_library(iotlab_i2c       STATIC iotlab_i2c/iotlab_i2c)
add_library(iotlab_i2c_slave STATIC iotlab_i2c/iotlab_i2c_slave)

