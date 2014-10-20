if ("${DRIVERS}" STREQUAL "stm32f1xx")
include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx)

# Add the executables

add_executable(simple_idle simple_idle)
target_link_libraries(simple_idle platform)


endif ("${DRIVERS}" STREQUAL "stm32f1xx")
