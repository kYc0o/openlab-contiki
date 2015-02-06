/* This file implements the pedometer (free fall detection) parameters
 *
 * \date April 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */
#include <stdio.h>
#include <math.h>
#include "freefall.h"
#include "activity.h"

/* {level_fall, min_dur_fall, level_inactive, epsilon_inactive, min_dur_inactive  }*/
free_fall_config_t FREEFALL_PARAM ={1.5, 15, 10.0, 5.0, 15} ;

void freefall_setparam(free_fall_config_t freefall_params) 
{
  /* Params verification */
  if (freefall_params.min_dur_fall < 0)
    freefall_params.min_dur_fall = 0;
  if (freefall_params.min_dur_inactive < 0)
    freefall_params.min_dur_inactive = 0;

  /* Set params */
  FREEFALL_PARAM.level_fall = freefall_params.level_fall ;
  FREEFALL_PARAM.min_dur_fall = freefall_params.min_dur_fall ;
  FREEFALL_PARAM.level_inactive = freefall_params.level_inactive ;
  FREEFALL_PARAM.epsilon_inactive = freefall_params.epsilon_inactive ;
  FREEFALL_PARAM.min_dur_inactive = freefall_params.min_dur_inactive ;    
}


void freefall(int k, float norm, int *state) 
{
  static int time_freefall, time_inactive;

  /* Initialisation */
  *state = NONDEF;
  if (k==0) {
    time_freefall = 0;
    time_inactive = 0;
  }
  
  /*
   1 - Threshold level (<1.5g) for freefall detection
  */ 
  if ( norm <= FREEFALL_PARAM.level_fall ) {
    time_freefall +=1 ; 
  } else {
    if (time_freefall > 0)
      time_freefall = 0 ;
  }
  /*
   2 - Threshold duration time (> 100 ms) for freefall detection
  */
  if (time_freefall >=  FREEFALL_PARAM.min_dur_fall ) {
    *state = FREEFALL;    
  }
  /*
   3 - State level detection (=10g +/- 5g)for inactivity detection
  */
  if ( *state == NONDEF ) {
    if ( (norm <= FREEFALL_PARAM.level_inactive + FREEFALL_PARAM.epsilon_inactive ) &&
	 (norm >= FREEFALL_PARAM.level_inactive - FREEFALL_PARAM.epsilon_inactive ) ) {
      time_inactive +=1 ;
    } else {
      time_inactive = 0;
    }
  } else {
    time_inactive = 0;
  }
  /*
   4 - Threshold duration time (> 100 ms) for inactivity detection
  */
   if (time_inactive >=  FREEFALL_PARAM.min_dur_inactive ) {
     *state =  INACTIVE;
   }
   /*
    5 - By default active
   */ 
   if (*state == NONDEF) {
     *state = ACTIVE;
   }
}
