if(${PLATFORM_HAS_CSMA})
add_executable(distributed_algorithm main computing radio_network)
target_link_libraries(distributed_algorithm platform mac_csma)
endif(${PLATFORM_HAS_CSMA})
