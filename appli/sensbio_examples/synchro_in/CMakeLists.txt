#
# This file is part of HiKoB Openlab. 
# 
if("${PLATFORM}" STREQUAL "agile-fox")

include_directories(
	${PROJECT_SOURCE_DIR}/drivers/cortex-m3
	${PROJECT_SOURCE_DIR}/drivers/stm32
	${PROJECT_SOURCE_DIR}/drivers/stm32f1xx)
	add_executable(synchro_in synchro_in)
	target_link_libraries(synchro_in platform)	
endif("${PLATFORM}" STREQUAL "agile-fox")