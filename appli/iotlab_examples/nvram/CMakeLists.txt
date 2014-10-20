if(${PLATFORM_HAS_N25XXX})
    add_executable(nvram_example main)
    target_link_libraries(nvram_example platform)
endif(${PLATFORM_HAS_N25XXX})

