/* This file implements the pedometer (rotation detection) parameters
 *
 * \date September 04, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */

#ifndef _CULBUTO_H
#define _CULBUTO_H

typedef	struct culbuto_config {
  float level_rot;
  int duration_rot;
} culbuto_config_t;

extern void culbuto(int k, float norm, int *state);

extern void culbuto_setparam(culbuto_config_t culbuto_params);

#endif
