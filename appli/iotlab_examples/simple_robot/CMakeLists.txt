if (PLATFORM STREQUAL "iotlab-m3")
add_executable(simple_robot main)
target_link_libraries(simple_robot platform)
endif (PLATFORM STREQUAL "iotlab-m3")

