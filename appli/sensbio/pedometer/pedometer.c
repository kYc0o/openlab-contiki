/* This file implements the pedometer parameters
 *
 * \date April 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */

#include <stdio.h>
#include <math.h>
#include "pedometer.h"

void pedometer_setparam(count_steps_config_t steps_params, 
			free_fall_config_t fall_params,
			culbuto_config_t culbuto_params) {

  countstep_setparam(steps_params);
  freefall_setparam(fall_params);
  culbuto_setparam(culbuto_params);
}


void pedometer(int k, float acc[3], float gyr[3],
	       int *step, int *state,
	       float *debug)
{
  float norm_acc;
  float norm_gyr;

  /*
   1 - Compute acceleration and gyro norm
  */
  norm_acc = acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2];
  norm_acc = sqrt(norm_acc);

  norm_gyr = gyr[0]*gyr[0] + gyr[1]*gyr[1] + gyr[2]*gyr[2];
  norm_gyr= sqrt(norm_gyr);
  /*
   2 - Compute step numbers
  */
  countstep(k, norm_acc, step);
  /*
   3 - Compute freefall/activity state
  */
  freefall(k, norm_acc, state);
  /*
   4 - Compute rotation/culbuto state
  */
  culbuto(k, norm_gyr, state);

  debug = 0 ;
}
