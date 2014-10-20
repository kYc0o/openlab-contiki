include_directories(
	${PROJECT_SOURCE_DIR}/drivers/cortex-m3
	${PROJECT_SOURCE_DIR}/drivers/stm32
	${PROJECT_SOURCE_DIR}/drivers/stm32f1xx)

# only run on A8 with a GPS
if (PLATFORM STREQUAL "iotlab-a8-m3")

add_executable(getTime getTime)
add_executable(getTimeRadioSnd getTimeRadioSnd)
add_executable(getTimeRadioRcv getTimeRadioRcv)
add_executable(getTimeStats getTimeStats)

target_link_libraries(getTime platform printf)
target_link_libraries(getTimeRadioSnd platform printf)
target_link_libraries(getTimeRadioRcv platform printf)
target_link_libraries(getTimeStats platform printf)

endif (PLATFORM STREQUAL "iotlab-a8-m3")
