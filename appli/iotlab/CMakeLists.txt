# Add the tasks directory
# iotlab specific code

if (PLATFORM MATCHES "iotlab-.*")

add_subdirectory(control_node)
add_subdirectory(idle_with_dc)

add_subdirectory(open_node_m3_autotest)

add_subdirectory(node_radio_characterization)

# Libraries
add_subdirectory(autotest_common)
add_subdirectory(lib)
add_subdirectory(periph)

endif (PLATFORM MATCHES "iotlab-.*")
