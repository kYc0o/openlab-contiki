if (PLATFORM STREQUAL "iotlab-m3" OR PLATFORM STREQUAL "iotlab-a8-m3")
    # only platform with INCLUDE_uxTaskGetStackHighWaterMark support
    add_executable(test_scanf_stack_usage scanf_stack_usage)
    target_link_libraries(test_scanf_stack_usage platform)
endif (PLATFORM STREQUAL "iotlab-m3" OR PLATFORM STREQUAL "iotlab-a8-m3")
