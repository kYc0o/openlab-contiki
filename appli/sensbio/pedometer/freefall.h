/* This file implements the pedometer (free fall detection) parameters
 *
 * \date April 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *          
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */

#ifndef _FREEFALL_H
#define _FREEFALL_H

typedef	struct free_fall_config {
  float level_fall;
  int min_dur_fall;
  float level_inactive;
  float epsilon_inactive;
  int min_dur_inactive;
} free_fall_config_t;

extern void freefall(int k, float norm, int *state);

extern void freefall_setparam(free_fall_config_t freefall_params);

#endif
