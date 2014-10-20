# Add the executables
if(${PLATFORM_HAS_INA226})
        add_executable(idle_with_dc idle_with_dc)
        target_link_libraries(idle_with_dc platform)
endif(${PLATFORM_HAS_INA226})
