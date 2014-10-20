/*
 * Idle program
 *
 */
#include "platform.h"

#if PLATFORM_HAS_ISL29020
#include "isl29020.h"
#endif // PLATFORM_HAS_ISL29020

#if PLATFORM_HAS_L3G4200D
#include "l3g4200d.h"
#endif  // PLATFORM_HAS_L3G4200D

#if PLATFORM_HAS_LSM303DLHC
#include "lsm303dlhc.h"
#endif  // PLATFORM_HAS_LSM303DLHC

#if PLATFORM_HAS_LPS331AP
#include "lps331ap.h"
#endif  // PLATFORM_HAS_LPS331AP

#include "flash.h"


void stop_hardware()
{
    leds_off(0xFF);

#if PLATFORM_HAS_ISL29020
    isl29020_powerdown();
#endif  // PLATFORM_HAS_ISL29020


#if PLATFORM_HAS_L3G4200D
    l3g4200d_powerdown();
#endif  // PLATFORM_HAS_L3G4200D

#if PLATFORM_HAS_LSM303DLHC
    lsm303dlhc_powerdown();
#endif

#if PLATFORM_HAS_LPS331AP
    lps331ap_powerdown();
#endif  // PLATFORM_HAS_LPS331AP


#if PLATFORM_HAS_N25XXX
    ;  // no powerdown for flash ?
#endif // PLATFORM_HAS_N25XXX


#if PLATFORM_HAS_PHY
    phy_reset();
#endif  // PLATFORM_HAS_PHY
// PLATFORM_HAS_RF231  included in phy for our needs

#if PLATFORM_HAS_I2C_EXTERNAL
    i2c_slave_configure(i2c_external, NULL);
#endif  // PLATFORM_HAS_I2C_EXTERNAL


    flash_set_wait_cycle(0);


    /*
     * TODO See in tests/lowpower for which mode to use
     *
     *   // set CPU to low power mode too
     *
     */




    //set(PLATFORM_HAS_SYSTICK 1)


}

int main()
{
	platform_init(); // Initialize the platform
	platform_run();  // Run
    /* idle hook will run "wfi" */
	return 0;
}
