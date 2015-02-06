/* This file implements the pedometer (rotation detection) parameters
 *
 * \date September 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */
#include <math.h>
#include "culbuto.h"
#include "activity.h"
#include "printf.h"

/* {level_rot }*/
culbuto_config_t CULBUTO_PARAM ={2000.0, 20} ;

void culbuto_setparam(culbuto_config_t culbuto_params)
{
/* Params verification */
  if (culbuto_params.level_rot < 0)
    culbuto_params.level_rot = 0;

/* Set params */
  CULBUTO_PARAM.level_rot = culbuto_params.level_rot;

}


void culbuto(int k, float norm, int *state)
{
  static int nb = 0;

  if (norm >= CULBUTO_PARAM.level_rot) {
	nb++;
	if (nb < CULBUTO_PARAM.duration_rot)
		return;
	*state |= ROTATION;
	//printf("gyr: %f\n", norm);
  }
  else {
	*state &= ~ROTATION;
	nb = 0;
  }

}
