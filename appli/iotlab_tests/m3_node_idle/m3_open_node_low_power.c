/*
 * put m3 open node in low power mode to reduce as much as possible power
 */


/*
 *
 * BETA CODE
 *
 */


#include <stdint.h>
#include "platform.h"
/*#include "isl29020.h"
#include "lps331ap.h"
#include "l3g4200d.h"
#include "lsm303dlhc.h"
#include "printf.h"*/

volatile int fake = 1;

int main()
{
    // Initialize the platform
    //platform_init();

    //stop the led
    leds_off(0xFF);

	while(fake)
	{
		asm("nop");
	}
    platform_init();
/*
    //Light sensor power-down
    isl29020_powerdown();

    //GYROMETER power-down
    if (l3g4200d_powerdown())
    	printf("GYROMETER l3g4200d power-down fail\n");

    //Pressure sensor power-down
    if (lps331ap_powerdown())
    	printf("PRESSURE sensor lps331ap power-down fail\n");

    //Accelerometer Magnetometer power-down
    if (lsm303dlhc_powerdown())
    	printf("Accelerometer Magnetometer sensors lsm303dlhc power-down fail\n");

	//in order to reduce consumption enter in stop mode based on deep-sleep mode
    while(1)
    {
    	pwr_enter_stop();
    }*/
    return 0;
}
