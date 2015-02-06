if(${PLATFORM_HAS_CSMA})
add_executable(distributed_computing main computing radio_network)
target_link_libraries(distributed_computing platform mac_csma)
endif(${PLATFORM_HAS_CSMA})
