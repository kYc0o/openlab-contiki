include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    ${PROJECT_SOURCE_DIR}/appli/iotlab/periph
    )

add_library(iotlab_serial     STATIC iotlab_serial)
add_library(iotlab_time       STATIC iotlab_time)
add_library(iotlab_leds_util  STATIC iotlab_leds_util)

add_library(iotlab_uid_num_table  STATIC
    iotlab_uid_num_hashtable
    iotlab_uid_num_hashtable_fcts)

add_library(zep_sniffer_format  STATIC zep_sniffer_format)
