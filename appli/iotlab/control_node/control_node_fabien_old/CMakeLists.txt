add_subdirectory(commands)


include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    ${PROJECT_SOURCE_DIR}/appli/iotlab/autotest_common
    ${PROJECT_SOURCE_DIR}/appli/iotlab/lib
    ${CMAKE_CURRENT_LIST_DIR}/commands
    )

link_directories(${PROJECT_BINARY_DIR}/appli/iotlab/control_node/commands)

add_executable(control_node_fabien_old  control_node constants)
target_link_libraries(control_node_fabien_old  iotlab_autotest_cn command printf platform)
