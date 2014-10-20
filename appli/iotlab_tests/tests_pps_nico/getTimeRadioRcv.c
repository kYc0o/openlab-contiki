/*
 * test_nico.c
 *
 *  Created on: Jul 24, 2013
 *      Author: Nicolas Turro <nicolas.turro.at.inria.fr>
 */

#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "iotlab-a8-m3/iotlab-a8-m3.h"
#include "printf.h"
#include "gpio.h"
#include "afio.h"
#include "nvic_.h"
#include "phy.h"
#include "unique_id.h"
#include "debug.h"
#include "soft_timer.h"

// our STM32F103 runs at 72Mhz, 72000065 is an average measurement from a external GPS PPS source. 
#define CYCLESPERSEC 72000065.0
#define CYCLES2NS 1000000000.0/CYCLESPERSEC

#define RADIO_CHANNEL 14
/** Enter RX state */
static void enter_rx(handler_arg_t arg);

/** Packet for receiving */
static phy_packet_t rx_packet;


#define START 0
#define MAGIC1 1
#define MAGIC2 2
#define TIMEOK 3
#define MAGICCHAR1 0x16
#define MAGICCHAR2 0x3

//  DWT_CYCNT counter is used instead of SysTick because it's a 32bits counter which overflows once a minute whereas
// SysTick overflows every 234 ms
volatile unsigned int *DWT_CYCCNT     = (volatile unsigned int *)0xE0001004; //address of the register
volatile unsigned int *DWT_CONTROL    = (volatile unsigned int *)0xE0001000; //address of the register
volatile unsigned int *SCB_DEMCR         = (volatile unsigned int *)0xE000EDFC; //address of the register

volatile uint32_t seconds=0;
volatile int A8comState=START;

typedef struct {
  uint32_t sec;
  uint32_t nsec;
} time_t;


// return the time elapsed since the beginning of the application
void getTime(time_t * curtime){
  // the seconds are updated by the PPShandler, nothing to do
  (*curtime).sec=seconds;
  // the nsec are computed from the number of cycles elapsed since the last second
  (*curtime).nsec=*DWT_CYCCNT*(double)CYCLES2NS;
}

void PPShandler(handler_arg_t arg) {
  // we recieve this interrupt each second from the GPS PPS
  // so we increment the number of seconds elapsed from the beginning
  seconds++;
  // and we reset the cycle counter
  *DWT_CYCCNT = 0;
}

static void rx_done_isr(phy_status_t status) { 
  time_t rcv_time;
  time_t delay;
  char snd_addr[20];
  char *pch;
  uint32_t snd_count;
  uint32_t snd_sec;
  uint32_t snd_nsec;
  
  switch (status) {
    case PHY_SUCCESS:
      getTime(&rcv_time);
      // Make sure data is terminated with a \0
      rx_packet.data[rx_packet.length] = 0;
      // Print the data
      pch=strtok((char *)rx_packet.data,",");
      strcpy(snd_addr,pch);
      pch=strtok(NULL,",");
      snd_count=atoi(pch);
      pch=strtok(NULL,".");
      snd_sec=atoi(pch);
      pch=strtok(NULL,",");
      snd_nsec=atoi(pch);
      //      printf("at %d.%09d, recieved packet %d sent from %s at %d.%09d with delta=%dns\n",rcv_time.sec,rcv_time.nsec,snd_addr,snd_sec,snd_nsec,rcv_time.nsec-snd_nsec);
      delay.sec=rcv_time.sec-snd_sec;
      delay.nsec=rcv_time.nsec-snd_nsec;
      printf("%u,%u\n",snd_count,delay.sec*1000000000+delay.nsec);
      break;
    default:
      // Bad packet
      //event_post_from_isr(EVENT_QUEUE_APPLI, enter_rx, NULL);
      log_debug("Bad packet\n");
      break;
    }
  enter_rx(NULL);
}

/** Enter radio RX state */
static void enter_rx(handler_arg_t arg) {
  // Set PHY IDLE, then enter RX
  phy_idle(platform_phy);
  phy_set_channel(platform_phy, RADIO_CHANNEL);
  // Prepare packet and start RX
  phy_prepare_packet(&rx_packet);
  phy_rx_now(platform_phy, &rx_packet, rx_done_isr);
}

/** Serial line character reading interrupt                      **/
/** Here, we read 6 bytes messages from the A8                   **/
/** 2 bytes magic characters followed by 4 bytes coding a uint32 **/
/** representing a time offset in seconds                        **/
void char_rx(handler_arg_t arg, uint8_t c)  {
  static union {
    uint8_t uint8array[4];
    uint32_t uint32;
  } A8buf;
  static int A8comCount;

  switch (A8comState)  {
  case START : 
    if (c==MAGICCHAR1) A8comState=MAGIC1;
    break;
  case MAGIC1 : 
    if (c==MAGICCHAR2) {
      A8comState=MAGIC2; 
      A8comCount=0;
    }
    else A8comState=START;
    break;
  case MAGIC2 :
    A8buf.uint8array[A8comCount]=c;
    A8comCount++;
    if (A8comCount==4) {
      seconds=A8buf.uint32;
      A8comState=TIMEOK;
    }
    break;
  case TIMEOK :
    if (c==MAGICCHAR1) A8comState=MAGIC1;
    break;    
  }
}

int main() {
  platform_init();
  soft_timer_init();

  printf("nico test \n===================\n");
  printf("Waiting time message from A8\n");
  
  uart_set_rx_handler(uart_print,  char_rx,NULL);

  while(A8comState!=TIMEOK){
    asm volatile ("nop");
  }
  printf("Got time from A8 : %us\n",seconds);

  // Init cycle counter
  *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
  *DWT_CYCCNT = 0; // reset the counter
  *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter

  // GPS PPS output is wired to GPIO PA3
  gpio_set_input(gpioA, GPIO_PIN_3);
  gpio_enable(gpioA);

  // link GPIO PA3 to IRQ_LINE_EXTI3
  afio_select_exti_pin(EXTI_LINE_Px3, AFIO_PORT_A);
  nvic_enable_interrupt_line(NVIC_IRQ_LINE_EXTI3);

  // set IRQ handler and configure call to RISING signal on PA3
  exti_set_handler(EXTI_LINE_Px3, PPShandler,NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px3, EXTI_TRIGGER_RISING);

  
  phy_reset(platform_phy);
  phy_set_channel(platform_phy, RADIO_CHANNEL);

  printf("Waiting for radio messages\n");
  enter_rx(NULL);
  platform_run();

  return 0;
}


