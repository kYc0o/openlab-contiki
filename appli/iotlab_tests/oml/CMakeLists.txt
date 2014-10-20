include_directories(
	${PROJECT_SOURCE_DIR}/platform/
	${PROJECT_SOURCE_DIR}/drivers/stm32/
	${PROJECT_SOURCE_DIR}/drivers/stm32f1xx/
	${PROJECT_SOURCE_DIR}/drivers/cortex-m3/
	${PROJECT_SOURCE_DIR}/lib/oml/
	)

add_executable(test_oml test_oml)
target_link_libraries(test_oml platform fiteco_lib_oml platform printf)
