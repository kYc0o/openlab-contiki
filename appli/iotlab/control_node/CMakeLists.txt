if (PLATFORM STREQUAL "iotlab-cn")

add_subdirectory(control_node)
# Non compatible anymore. Disable as I don't want to maintain it anymore
# add_subdirectory(control_node_fabien_old)
# add_subdirectory(controlnode_clement)

endif (PLATFORM STREQUAL "iotlab-cn")
