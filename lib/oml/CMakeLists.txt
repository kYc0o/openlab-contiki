include_directories(
	${PROJECT_SOURCE_DIR}/platform/
	${PROJECT_SOURCE_DIR}/drivers/stm32/
	${PROJECT_SOURCE_DIR}/drivers/stm32f1xx/
	${PROJECT_SOURCE_DIR}/drivers/cortex-m3/
	)
add_library(fiteco_lib_oml STATIC omlc comm omlcore omlmp)
