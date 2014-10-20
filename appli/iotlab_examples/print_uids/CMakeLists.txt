include_directories(${PROJECT_SOURCE_DIR}/appli/iotlab/lib)

if (PLATFORM STREQUAL "iotlab-m3" OR PLATFORM STREQUAL "iotlab-a8-m3")
add_executable(print_uids print_uids)
target_link_libraries(print_uids iotlab_uid_num_table platform)
endif (PLATFORM STREQUAL "iotlab-m3" OR PLATFORM STREQUAL "iotlab-a8-m3")
