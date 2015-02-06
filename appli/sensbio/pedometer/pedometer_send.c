/*  This file is the firmware of the sensor node sending data
 *
 * \date Oct 10, 2013
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2014 INRIA
 */
#include <stdint.h>
#include <strings.h> // bcopy
#include <math.h>
#include "platform.h"
#include "printf.h"
#include "lsm303dlhc.h"
#include "l3g4200d.h"
#include "event.h"
#include "soft_timer.h"
#include "mac_csma.h"

#include "msg.h"
#include "activity.h"

/* Period of the pedometer counter computation */
#define TX_PERIOD soft_timer_ms_to_ticks(5)
/* times of computation before send a result */
/* period in sec = (TX_PERIOD_MSEC=5 x TX_COMPUTE) / 1000 */
#define TX_COMPUTE  200
/* number of pedomter_data_t entries stored (max) */
#define CACHE_SIZE  100 

/** Handler to be called from the event task */
static void  compute_and_sometimes_send(handler_arg_t arg);
/** Static mac_handler_t packet_sent(handler_arg_t arg);*/
//static mac_handler_t packet_received(uint8_t packet[], uint16_t length, uint16_t src_addr);
/** Computation of criteria from sensor information */
static void compute_criteria(int *steps, int *activity);
/** Software Timer to periodically send a packet */
static soft_timer_t tx_timer;
/** Message to be sent **/
static pack_t msg;
/** Global counters structure */
typedef struct TypCounters {
  /* software status */
  uint8_t status;
  /* period for packet sending */
  uint32_t period;
  /* index incremented each time pedometer data is ready */
  uint32_t tindex;
  /* index incremented at each criteria computation */
  uint32_t index;
  /* local index incremented at each computation between 2 packet sending*/
  uint16_t lindex;
} TypCounters;

TypCounters counters = {NO_STATE, 1, 0, 0, 0};

static int localisation = 0;

struct {
	pedometer_data_t data[CACHE_SIZE];
	pedometer_data_t *curr_send;
	pedometer_data_t *curr_write;
} cache;

static void cache_init()
{
	cache.curr_write = cache.data;
	cache.curr_send  = cache.data;
}

static void cache_roll_send() {
	cache.curr_send ++;
	if (cache.curr_send == cache.data+CACHE_SIZE)
		cache.curr_send = cache.data;
}

static void cache_roll_write() {
	cache.curr_write ++;
	if (cache.curr_write == cache.data+CACHE_SIZE)
		cache.curr_write = cache.data;
	if (cache.curr_write == cache.curr_send)
		cache_roll_send();
}

static void do_send()
{
	printf("Sending = %d %d %d %d 0x%x\r\n",
		msg.count,
		msg.data.pedometer_data.time,
		msg.data.pedometer_data.steps,
		msg.data.pedometer_data.activity,
		msg.data.pedometer_data.neighbourhood);
	mac_csma_data_send(ADDR_BROADCAST,(uint8_t *) &msg,  sizeof(msg));
}

static void send_message() {
	bcopy(cache.curr_send, &msg.data.pedometer_data, sizeof(pedometer_data_t));
	msg.count++;
	msg.cmd = PCK_DATA;
	event_post(EVENT_QUEUE_APPLI, do_send, (handler_arg_t) 0);
}

static void fake_init();

int main()
{
    // Initialize the platform
    platform_init();
    event_init();
    soft_timer_init();
   
    cache_init();

    /* Initialize the Accelerometer */
    printf("# Initializing LSM303DLHC...\n");
    lsm303dlhc_powerdown();
    printf("# Setting LSM303DLHC accelerometer\n");
    lsm303dlhc_acc_config(LSM303DLHC_ACC_RATE_400HZ, 
			  LSM303DLHC_ACC_SCALE_16G, 
			  LSM303DLHC_ACC_UPDATE_ON_READ);
    /* Initialize the Gyrometer */
    l3g4200d_gyr_config(L3G4200D_200HZ, L3G4200D_2000DPS, true);

    /* Initialize the Mac csma layer */
    mac_csma_init(RADIO_CHANNEL, PHY_POWER_0dBm);

    /* Create a periodic packet computation & sending */
    soft_timer_set_handler(&tx_timer, compute_and_sometimes_send, NULL);
    soft_timer_start(&tx_timer, TX_PERIOD, 1);

    /* Initialize variables */
    msg.count=0;
    counters.status = NO_STATE; 
    /* Orange and green LED on */
    leds_on(LED_0);
    leds_on(LED_1);

    if (0) fake_init(); /* for debug */

    /* Run */
    printf("FOX_HIKOB_PEDOMETER:connected\n");
    platform_run();
    return 0;
}

static void send_ack(handler_arg_t arg)
{
    mac_csma_data_send(ADDR_BROADCAST,(uint8_t *) &msg,  sizeof(msg));
}

void mac_csma_data_received(uint16_t src_addr, const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
  const pack_t *pck = (const pack_t *) data;

  printf("0x%x: %d %d %d\r\n", src_addr,pck->count, pck->cmd, pck->data.param);

  switch (pck->cmd) {
  case PCK_INIT: 
    /* Orange LED on */
    leds_off(LED_0);
    counters.status = INIT;
    counters.index = 0;  
    counters.lindex = 0; 
    counters.tindex = 0; 
    counters.period = TX_COMPUTE * pck->data.config.period;
    if (pck->data.config.params_set)
        pedometer_setparam(
		pck->data.config.count_steps_config,
		pck->data.config.free_fall_config,
		pck->data.config.culbuto_config);
    /* Return ack */
    msg.count = 0;
    msg.cmd = PCK_ACK;
    msg.data.param = PCK_INIT;
    event_post(EVENT_QUEUE_APPLI, send_ack, (handler_arg_t) 0);
    break;
  case PCK_RUN:
    /* Green LED will blink */
    leds_off(LED_1);
    counters.status = RUN; 
    /* Return ack */
    msg.count = 0;
    msg.cmd = PCK_ACK;
    msg.data.param = PCK_RUN;
    event_post(EVENT_QUEUE_APPLI, send_ack, (handler_arg_t) 0);
    break;
  case PCK_ACK:
    /* Data packet received by dongle Ack */
    localisation = src_addr;
    cache_roll_send();
    if (cache.curr_send != cache.curr_write)
        send_message();
    break;
  case PCK_BEAM:
    /* Beamer packet, update localisation */
    localisation = src_addr;
    break;
  case PCK_DATA:
    /* Data packet ignored */
    break;
  }
}

static void fake_init() {
  pack_t pck;

  pck.cmd = PCK_INIT;
  pck.data.config.period = 1;
  pck.data.config.params_set = 0;
  mac_csma_data_received(0, (const uint8_t*)&pck, sizeof(pack_t), 0, 0);
  pck.cmd = PCK_RUN;
  mac_csma_data_received(0, (const uint8_t*)&pck, sizeof(pack_t), 0, 0);
}

static void compute_and_sometimes_send(handler_arg_t arg) {

  static int mem_activity;

 if (counters.status == RUN) { 
   int steps, activity;
   compute_criteria(&steps, &activity);
   /* Store activity by priority */
   mem_activity = mem_activity | activity;

   if (counters.lindex == counters.period) {
     printf("-> %d %d %d\n", counters.tindex, activity, mem_activity);
     /* Green LED blinks */
     leds_toggle(LED_0);
     counters.lindex = 0;
     /* feed cache */
     cache.curr_write->time = counters.tindex;
     cache.curr_write->steps = steps;
     cache.curr_write->activity = mem_activity;
     cache.curr_write->neighbourhood = localisation;
     cache_roll_write();
     /* update time index */
     counters.tindex ++;
     /* send message */
     send_message();
     /* advertize connectivity state */
     localisation ? leds_on(LED_1) : leds_off(LED_1);
     /* reset localisation, will be set on next ack */
     localisation = 0; 
     /* reset activity memory */
     mem_activity = INACTIVE;
   } else {
     counters.lindex++;
   }
 }
 else {
   mem_activity  = INACTIVE;
 }

  return ;
}


static void compute_criteria(int *step, int *activity) {
  int16_t a[3];
  float acc[3];
  int16_t w[3];
  float gyro[3];
  int i;
  float peak;

  /* Read accelerometers */
  lsm303dlhc_read_acc(a);
  /* Read gyrometers */
  l3g4200d_read_rot_speed(w);
  /* Conversion */
  for (i=0; i < 3; i++) {
    acc[i] = a[i] * ACC_RES * GRAVITY;
    gyro[i] = w[i];
  }
  /* Compute state */
  pedometer(counters.index, acc, gyro, step, activity, &peak);
  counters.index ++;
}

