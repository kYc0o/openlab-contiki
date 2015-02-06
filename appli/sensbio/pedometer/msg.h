/* Constants definition
 *
 * \date Oct 10, 2013
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */

/* CHANNEL_23 and 25 for PTL experiment */
#define RADIO_CHANNEL 23
#define ACC_RES (12e-3)  // The resolution is 12mg for the +/-16g scale
#define MAG_RES (1/670.) // The resolution is 1/670 for the +/-2.5gauss scale

#define GRAVITY 9.81
#define ADDR_BROADCAST 0xFFFF

#include "pedometer.h"

enum packet_type {PCK_ACK=0, PCK_INIT=1, PCK_RUN=2, PCK_BEAM=3, PCK_DATA=4};

typedef struct pedometer_params {
	int period;
	int params_set;
        count_steps_config_t count_steps_config;
        free_fall_config_t  free_fall_config;
        culbuto_config_t  culbuto_config;
} pedometer_params_t;

typedef struct pedometer_data {
	int time;
	int steps;
	int activity;
	int neighbourhood;
} pedometer_data_t;

typedef struct __attribute__ ((packed))
{
  uint16_t count;
  enum packet_type cmd;
  union {
	pedometer_data_t pedometer_data;
	pedometer_params_t config;
	int param;
  } data;
} pack_t;

/** Software status **/
enum status_type {NO_STATE=0, INIT=1, RUN=2, BEAM=3};
