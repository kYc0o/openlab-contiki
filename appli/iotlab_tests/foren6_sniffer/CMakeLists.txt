if(${PLATFORM_HAS_PHY})
    add_executable(foren6_sniffer main foren6_sniffer)
    target_link_libraries(foren6_sniffer platform)
endif(${PLATFORM_HAS_PHY})
