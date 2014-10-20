#
# This file is part of HiKoB Openlab. 
# 
if("${PLATFORM}" STREQUAL "agile-fox")

include_directories(
	${PROJECT_SOURCE_DIR}/drivers/cortex-m3
	${PROJECT_SOURCE_DIR}/drivers/stm32
	${PROJECT_SOURCE_DIR}/drivers/stm32f1xx)
	add_executable(synchro_out synchro_out)
	target_link_libraries(synchro_out platform)	
endif("${PLATFORM}" STREQUAL "agile-fox")