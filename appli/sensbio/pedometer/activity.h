/* This file defines the pedometer "activity" values,
 * used as indicators (bitfield) for detected high-level states.
 *
 * \date September 01, 2014
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */

#ifndef _ACTIVITY_H
#define _ACTIVITY_H

typedef enum {
	NONDEF=-1,
	INACTIVE=0,
	ACTIVE=1,
	FREEFALL=2,
	ROTATION=4
} activity_t;

#endif
