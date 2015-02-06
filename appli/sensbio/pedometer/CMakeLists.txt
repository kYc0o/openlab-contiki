#
# This file is part of HiKoB Openlab. 
# 

if(${PLATFORM_HAS_LSM303DLHC})
	add_executable(pedometer_send pedometer_send pedometer freefall countstep culbuto)
	target_link_libraries(pedometer_send platform lsm303dlhc mac_csma)	
	add_executable(pedometer_receive pedometer_receive)
	target_link_libraries(pedometer_receive platform lsm303dlhc mac_csma)
endif(${PLATFORM_HAS_LSM303DLHC})
