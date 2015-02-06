/* This file implements the pedometer (counting steps) parameters
 *
 * \date April 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */
#include <stdio.h>
#include <math.h>
#include "countstep.h"


/* {window_size, peak_tempo, threshold */
count_steps_config_t STEP_PARAM ={25, 50, 11.0} ;

void countstep_setparam(count_steps_config_t steps_params)
{
  /* Params verification */
  if (steps_params.window_size < 0)
    steps_params.window_size = 0;
  else if (steps_params.window_size >  WINDOWS_MAX )
    steps_params.window_size =  WINDOWS_MAX;
  if (steps_params.peak_tempo < 0)
    steps_params.peak_tempo = 0;
  /* Set params */
  STEP_PARAM.window_size = steps_params.window_size;
  STEP_PARAM.peak_tempo  = steps_params.peak_tempo;
  STEP_PARAM.threshold   = steps_params.threshold;
}

void countstep(int k, float norm, int *pstep)
{
  static float sum, norm_trace[WINDOWS_MAX], moy, dmoy, moy_trace, peak;
  static short int sign, sign_trace, k_trace, step;
  /* 
   1 - Moving Average 
  */
  if (k==0) {
    sum = norm;
    moy = norm;
    step = 0;
    k_trace = 0;
  } 
  else if (k < STEP_PARAM.window_size) {
    sum = sum + norm;
    moy = sum / (k+1);
  }
  else {
    sum = sum + norm - norm_trace[k%STEP_PARAM.window_size];
    moy = sum / STEP_PARAM.window_size;
  }
  /*
   2 - Search Peak 
  */
  /* Compute derivative signal */
  if (k==0) {
    k_trace = 0;
    dmoy = 0.0;
    if (dmoy >=0 )
      sign_trace = 1;
    peak = 0;
  }
  else { 
    /* Compute derivative signal */
    dmoy = moy - moy_trace;
  }
  /* Compute the sign of the derivative signal */
  if (dmoy >=0 ) 
    sign = 1;
  else
    sign = -1; 
  /* Compute the switch of the sign with a signal threshold */
  if ( (sign_trace == 1) & (sign == -1) & (moy > STEP_PARAM.threshold) 
       & ( k > (k_trace + STEP_PARAM.peak_tempo) ) ) {
    peak = moy; 
    step ++;
    k_trace = k;
  }
  else
    peak = 0.0;

  if ( peak > 0)
    printf("PEAK %d %d %f %f \n",k, step, moy, dmoy);
  
  /* return value */
  *pstep = step;
  
 /* Store old values */
  norm_trace[k%STEP_PARAM.window_size] = norm; 
  moy_trace = moy;
  sign_trace = sign;
}
