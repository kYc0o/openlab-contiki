/* This file implements the pedometer parameters
 *
 * \date April 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */
#ifndef _PEDOMETER_H
#define _PEDOMETER_H

#include "countstep.h"
#include "freefall.h"
#include "culbuto.h"



void pedometer(int k, float acc[3], float gyr[3],
	       int *step, int *state,
	       float *debug);

void pedometer_setparam(count_steps_config_t steps_params, 
			free_fall_config_t fall_params,
			culbuto_config_t culbuto_params);

#endif
