/* This file implements the pedometer (counting steps) parameters
 *
 * \date April 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */
#ifndef _COUNTSTEP_H
#define _COUNTSTEP_H

#define WINDOWS_MAX 100

typedef	struct count_steps_config {
  int window_size;
  int peak_tempo;
  float threshold;
} count_steps_config_t;

extern void countstep(int k, float norm, int *state);

extern void countstep_setparam(count_steps_config_t steps_params);

#endif
