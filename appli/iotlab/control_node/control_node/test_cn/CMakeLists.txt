# Add the test directory


if(${PLATFORM_HAS_INA226})
    include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/

    ${PROJECT_SOURCE_DIR}/appli/iotlab/periph
    ${PROJECT_SOURCE_DIR}/appli/iotlab/autotest_common
    ${PROJECT_SOURCE_DIR}/appli/iotlab/lib
    ${PROJECT_SOURCE_DIR}/appli/iotlab/control_node/control_node/
    )
    add_executable(test_cn
        main
        ../cn_i2c
        )
    target_link_libraries(test_cn
        platform
        packet
        fiteco_lib_gwt
        iotlab_autotest_cn
        iotlab_i2c_slave
        iotlab_serial
        iotlab_time
        iotlab_leds_util
        )
endif(${PLATFORM_HAS_INA226})
