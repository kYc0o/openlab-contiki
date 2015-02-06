/* This file is the firmware of the sink node receiving data
 *
 * \date Oct 10, 2013
 * \authors <roger.pissard.at.inria.fr>, <olivier.fambon.at.inria.fr>
 *
 * INRIA PTL HikoB-Pedometer demo
 * Copyright (C) 2013 INRIA
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h> // memcpy
#include "platform.h"
#include "printf.h"
#include "scanf.h"

#include "phy.h"
#include "event.h"
#include "soft_timer.h"
#include "queue.h"

#include "mac_csma.h"
#include "msg.h"

#define LINE_MAX 100

/** Software Timer to periodically send a request init */
static soft_timer_t tx_timer;
#define TX_PERIOD soft_timer_s_to_ticks(1)
/** Handler to be called from the event task */
static void periodic_task(handler_arg_t arg);
/** Handler to receive order on uart */
static void char_rx(handler_arg_t arg, uint8_t c);
/** Message command to be sent **/
static pack_t msgcmd;

static uint16_t glob_status;
static pedometer_params_t glob_config;

int main()
{
  /* Initialize the platform */
    platform_init();
    event_init();
    soft_timer_init();
    glob_status = NO_STATE;

    /* Initialize the Mac csma layer */
    mac_csma_init(RADIO_CHANNEL, PHY_POWER_0dBm);

    /* Uart initialisation */
    uart_set_rx_handler(uart_print, char_rx, NULL);

    /* Periodic task for blinking leds */
    soft_timer_set_handler(&tx_timer, periodic_task, NULL);
    soft_timer_start(&tx_timer, TX_PERIOD, 1);

    /* Run */
    printf("FOX_USB_PEDOMETER:connected\n");
    platform_run();

    return 0;
}

/* Blinking LEDs */
static void  periodic_task(handler_arg_t arg) {
  leds_toggle(LED_0);
  leds_toggle(LED_1);
}

/* Messages timeout management */
static void timeout_error()
{
	printf("FOX_USB_PEDOMETER:error_send_timeout\n");
}

static soft_timer_t watchdog_timer;
static void watchdog_start()
{
        soft_timer_set_handler(&watchdog_timer, (handler_t)timeout_error, NULL);
        soft_timer_start(&watchdog_timer, soft_timer_ms_to_ticks(1000), 0);
}

static void watchdog_stop()
{
        soft_timer_stop(&watchdog_timer);
}

/* Message sending, with timeout, expects ack */
static void send_message()
{
  watchdog_start();  
  mac_csma_data_send(ADDR_BROADCAST,(uint8_t *) &msgcmd, sizeof(msgcmd));
  /* Wait for ack response in mac_csma_data_indication */    
}

/* Ack messages */
static void send_ack(handler_arg_t arg)
{
  mac_csma_data_send(ADDR_BROADCAST,(uint8_t *) &msgcmd, sizeof(msgcmd));
}

/* resend sometimes causes dups; filter them out based on last timestamp */ 
static int last_data_time = -1;

void handle_dongle_mode(uint16_t src_addr, const pack_t *msg)
{
  if ( msg->cmd ==  PCK_ACK ) {
    watchdog_stop();
    /* Ack cmd message */
    switch (msg->data.param) {
    case PCK_INIT :
       /* Stop blinking */ 
      soft_timer_stop(&tx_timer);
      glob_status = INIT;
      leds_on(LED_0); 
      leds_off(LED_1); 
      printf("FOX_USB_PEDOMETER:init_ok\n");
      break;
    case PCK_RUN :
      glob_status = RUN;
      leds_off(LED_0); 
      leds_on(LED_1); 
      printf("FOX_USB_PEDOMETER:run_ok\n");
      break;
    default:
      printf("FOX_USB_PEDOMETER:error_ack\n");
    }
  }
  else 
    if ( (glob_status==RUN) && ( msg->cmd ==  PCK_DATA ) ) {
      leds_toggle(LED_1);
      if (msg->data.pedometer_data.time == last_data_time) {
	;//printf("DUP detected: last_data_time=%d\n", last_data_time);
      }
      else {
      last_data_time = msg->data.pedometer_data.time;
      printf("0x%x: %d %d %d %d 0x%x\r\n",
		src_addr,
		msg->count,
		msg->data.pedometer_data.time,
		msg->data.pedometer_data.steps,
		msg->data.pedometer_data.activity,
		msg->data.pedometer_data.neighbourhood);
      }
      /* Return ack */
      msgcmd.cmd=PCK_ACK;
      event_post(EVENT_QUEUE_APPLI, send_ack, (handler_arg_t) 0);
    }
}

void handle_beamer_mode(uint16_t src_addr, const pack_t *msg)
{
  if (msg->cmd != PCK_DATA)
	return;
  msgcmd.cmd = PCK_BEAM;
  event_post(EVENT_QUEUE_APPLI, send_ack, (handler_arg_t) 0);
}

/* Reception of a radio message */
void mac_csma_data_received(uint16_t src_addr, const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
  const pack_t *msg = (const pack_t *) data;

  if (glob_status == BEAM)
	handle_beamer_mode(src_addr, msg);
  else
	handle_dongle_mode(src_addr, msg);
}

/* Initialisation of the application */
static void pedometer_init(pedometer_params_t *config) {
  /* Send a message to the pedometer node */
  msgcmd.cmd=PCK_INIT;
  msgcmd.count=0;
  memcpy(&msgcmd.data.config, config, sizeof(pedometer_params_t));
  send_message();
}

/* Launch the acquisition */
static void pedometer_run() { 
  /* Send a message to the pedometer node */
  msgcmd.cmd=PCK_RUN;
  msgcmd.count=0;
  msgcmd.data.param=0;
  send_message();
}

static void pedometer_beamer_mode() {
	glob_status = BEAM;
	soft_timer_stop(&tx_timer);
	leds_off(LED_0); 
	leds_off(LED_1); 
	printf("FOX_USB_PEDOMETER:beam_mode_on\n");
}

void parse_parameters(const char* params_str, pedometer_params_t *config)
{
	memset(config, 0, sizeof(pedometer_params_t));
	int ret = sscanf(params_str,
		"%d %d %d %f %f %d %f %f %d",
		&config->period,
		&config->count_steps_config.window_size,
		&config->count_steps_config.peak_tempo,
		&config->count_steps_config.threshold,
		&config->free_fall_config.level_fall,
		&config->free_fall_config.min_dur_fall,
		&config->free_fall_config.level_inactive,
		&config->free_fall_config.epsilon_inactive,
		&config->free_fall_config.min_dur_inactive
	);
	if (params_str[0] == 0) {
		config->period = 1; // default value
		return;
	}
	switch (ret) {
	case  0:
	case -1:
		config->period = 0; // indicate error
		break;
	case 1:
		break;
	default:
		if (ret < 9)
			config->period = 0; // indicate error
		else
			config->params_set = 1;
	}
}

/* Check the line from the UART, analyze and launch an action*/
static void checkline(uint8_t *cmd) {
  switch(cmd[0]) {
  case 'O':
    printf("FOX_USB_PEDOMETER:open_ok\n");
    break;
  case 'I':
    parse_parameters((const char*)++cmd, &glob_config);
    if (glob_config.period > 0) {
      pedometer_init(&glob_config);
    } else {
      printf("FOX_USB_PEDOMETER:error_init\n");
    }
    break;
  case 'R':
    if (glob_status==INIT)
      pedometer_run();
    else
      printf("FOX_USB_PEDOMETER:error_state_run\n");
    break;
  case 'B':
    pedometer_beamer_mode();
    break;
  case 'F':
    glob_status = RUN;
    leds_off(LED_0); 
    leds_on(LED_1); 
    printf("FOX_USB_PEDOMETER:force_run_ok\n");
    break;
  case 'Z':
    glob_status = INIT;
    leds_off(LED_0); 
    leds_on(LED_1); 
    printf("FOX_USB_PEDOMETER:force_sleep_ok\n");
    break;
  default:
    printf("FOX_USB_PEDOMETER:error_config_msg\n");
  }
}

/* Reception of a char on UART and store it in 'cmd' */
static void char_rx(handler_arg_t arg, uint8_t c)
{
  static uint16_t index = 0;
  static uint8_t cmd[LINE_MAX];

  if (index==0) {
    /* char to indicate the begin of a message */
    if (c=='F') 
      index++;
  }
  else {
    if (index > LINE_MAX) {
      printf("FOX_USB_PEDOMETER:error_config_msg\n");
      index = 0;
    } else {
      if (c!='\n') {
	cmd[index-1] = c;
	index ++;
      }
      else {
	cmd[index-1]=0;
	if (cmd[index-2]=='\r')
	  cmd[index-2]=0;
	checkline(cmd);
	index = 0;
      }
    }
  }
}
