include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    ${PROJECT_SOURCE_DIR}/appli/iotlab/lib
    ${PROJECT_SOURCE_DIR}/appli/iotlab/periph
    )

add_library(iotlab_autotest_cn STATIC iotlab_autotest_cn)
target_link_libraries(iotlab_autotest_cn iotlab_gpio)

add_library(iotlab_autotest_on STATIC  iotlab_autotest_on)
target_link_libraries(iotlab_autotest_on iotlab_gpio)

