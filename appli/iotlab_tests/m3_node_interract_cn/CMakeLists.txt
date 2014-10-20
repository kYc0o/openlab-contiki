include_directories(${PROJECT_SOURCE_DIR}/appli/iotlab/periph)

if (PLATFORM MATCHES "iotlab-.*")
    # Requires the 'iotlab-' libs
    add_executable(m3_node_interract_cn main)
    target_link_libraries(m3_node_interract_cn iotlab_gpio iotlab_i2c platform)
endif (PLATFORM MATCHES "iotlab-.*")
