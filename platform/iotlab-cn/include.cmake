set(OOCD_TARGET stm32f1x)

set(PLATFORM_OOCD_ITF ${PROJECT_SOURCE_DIR}/platform/scripts/iotlab-cn.cfg)
set(MY_C_FLAGS "${MY_C_FLAGS} -DIOTLAB_CN")

set(LINKSCRIPT ../scripts/stm32f103rey6.ld)

set(DRIVERS stm32f1xx)

set(PLATFORM_RAM_KB 64)

# Set the flags to select the application that may be compiled
set(PLATFORM_HAS_INA226 1)
set(PLATFORM_HAS_RF231 1)

set(PLATFORM_HAS_PHY 1)
set(PLATFORM_HAS_SYSTICK 1)
set(PLATFORM_HAS_CSMA 1)
set(PLATFORM_HAS_TDMA 1)
set(PLATFORM_HAS_I2C_EXTERNAL 1)

#add i2c slave support
set(PLATFORM_HAS_I2C_SLAVE 1)
set(MY_C_FLAGS "${MY_C_FLAGS} -DI2C__SLAVE_SUPPORT")

# -Werror by default
set(MY_C_FLAGS "${MY_C_FLAGS} -Werror")


#
# Control Node configuration
#     Hack because libs cannot be configured 'dynamically'
#

# Increase the event queue size and disable 'HALT' on post failure
set(MY_C_FLAGS "${MY_C_FLAGS} -DEVENT_QUEUE_LENGTH=64 -DEVENT_HALT_ON_POST_ERROR=0")

# serial pkt size == 256: (sync | len | payload) with len <= 255)
set(MY_C_FLAGS "${MY_C_FLAGS} -DPACKET_MAX_SIZE=256")

include(${PROJECT_SOURCE_DIR}/platform/include-cm3.cmake)
