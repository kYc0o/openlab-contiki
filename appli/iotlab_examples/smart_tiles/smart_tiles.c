/*
 * Smart tiles
 *
 */

#include <platform.h>
#include <printf.h>
#include <string.h>
#include <math.h>

#include "lsm303dlhc.h"
#include "event.h"
#include "detectpeak.h"

#undef  RT_PRINT
//#define RT_PRINT 1

extern void radio_recv_init();

static soft_timer_t tx_timer;
/* Period of the sensor acquisition datas */
#define ACQ_PERIOD soft_timer_ms_to_ticks(5)
/* times of computation before transmit a result */
/* period in sec = (ACQ_PERIOD=5 x TX_COMPUTE) / 1000 */
#define TX_PERIOD 200

//event handler
static void handle_ev(handler_arg_t arg);

#define ACC_RES (1e-3)  // The resolution is 1 mg for the +/-2g scale
#define GYR_RES (8.75e-3)  // The resolution is 8.75mdps for the +/-250dps scale
#define GRAVITY 9.81
#define CALIB_PERIOD 100 // period in sec = CALIB_PERIOD=5 x TX_COMPUTE) / 1000 

/** Global counters structure */
typedef struct TypCounters {
 /* index incremented at each criteria computation */
  uint32_t index;
  /* local index incremented at each computation between 2 packet sending*/
  uint16_t lindex;
} TypCounters;

TypCounters glob_counters = {0, 0};

static count_peak_config_t PEAKACC_TRACE;
count_peak_config_t PEAKMAG_TRACE;

static void hardware_init()
{
    // Openlab platform init
    platform_init();
    event_init();
    soft_timer_init(); 
    // Switch off the LEDs
    leds_off(LED_0 | LED_1 | LED_2);
    // LSM303DLHC accelero sensor initialisation
    lsm303dlhc_powerdown();
    lsm303dlhc_acc_config(
			  LSM303DLHC_ACC_RATE_400HZ,	
			  LSM303DLHC_ACC_SCALE_2G,	
			  LSM303DLHC_ACC_UPDATE_ON_READ);
    // LSM303DLHC magneto sensor initialisation
    lsm303dlhc_mag_config(LSM303DLHC_MAG_RATE_220HZ,
                          LSM303DLHC_MAG_SCALE_2_5GAUSS, 
			  LSM303DLHC_MAG_MODE_CONTINUOUS,
                          LSM303DLHC_TEMP_MODE_ON);
    // Set peak detection parameters: windows size, peak_tempo, threshold */
    peak_setparam(&PEAKACC_TRACE,10, 50, 1.01);   // 1.00: too sensitive
    peak_setparam(&PEAKMAG_TRACE,100, 100, 1.05); // 1.10: must be on tile
    // Initialize a openlab timer
    soft_timer_set_handler(&tx_timer, handle_ev, NULL);
    soft_timer_start(&tx_timer, ACQ_PERIOD, 1);
    // start radio receiver
    radio_recv_init();
}

int main()
{
	hardware_init();
	platform_run();
	return 0;
}

static void handle_ev(handler_arg_t arg)
{
  int16_t a[3];
  int16_t m[3];
  int16_t i;
  float af[3], mf[3];
  float accpeak[2];
  float magpeak[2];
  static float accnorm, magnorm;
  static float accscale, magscale;
  float accnormk, magnormk;

  /* Read accelerometers */ 
  lsm303dlhc_read_acc(a);
  /* Read magnetometers */ 
  lsm303dlhc_read_mag(m);
  /* Sensors calibration during CALIB_PERIOD*/
  if (glob_counters.index <= CALIB_PERIOD) {
    /* Scale Accelero and magneto*/
    accscale = 1.0;
    magscale = 1.0;
    /* first index */
    if (glob_counters.index == 0)
      {
	accnorm = 0.0;
	magnorm = 0.0;
      }
    /* computation */
    accnormk = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
    accnorm += sqrt(accnormk);
    magnormk = m[0] * m[0] + m[1] * m[1] + m[2] * m[2];
    magnorm += sqrt(magnormk);
    /* last index */
    if (glob_counters.index == CALIB_PERIOD) {
      accscale = CALIB_PERIOD / accnorm;
      magscale = CALIB_PERIOD / magnorm;
      printf("CALIB Acc = %f %f\n",accnorm, accscale);
      printf("CALIB Mag = %f %f\n",magnorm, magscale);
    }
    glob_counters.index++;
  } /* After calibration */
  else {
    /* Conversion */
    for (i=0; i < 3; i++) {
      af[i] = a[i] * accscale; 
      mf[i] = m[i] * magscale;
    } 
    /* Peaks detection after calibration*/ 
    peak_detect(&PEAKACC_TRACE, glob_counters.index, af, accpeak); 
    peak_detect(&PEAKMAG_TRACE, glob_counters.index, mf, magpeak);

    glob_counters.index++;
    /* Printing */
    if (accpeak[1] > 0.0) {
      printf("AccPeak;%f;0.0;0.0\n", accpeak[1]);
    }
    if (magpeak[1] > 0.0) {
      printf("MagPeak;%f;0.0;0.0\n", magpeak[1]);
    }

#ifdef RT_PRINT
    printf("Acc;%f;%f;%f\n", af[0], af[1], af[2]);
    printf("Mag;%f;%f;%f\n", mf[0], mf[1], mf[2]);
#endif

    if (glob_counters.lindex == TX_PERIOD) {
#ifndef RT_PRINT
      printf("Acc;%f;%f;%f\n", af[0], af[1], af[2]);
      printf("Mag;%f;%f;%f\n", mf[0], mf[1], mf[2]);
#endif
      glob_counters.lindex=0;
    }
    else {
      glob_counters.lindex++;
    }
  }

}
