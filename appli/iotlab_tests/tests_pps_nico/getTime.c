/*
 * test_nico.c
 *
 *  Created on: Jul 24, 2013
 *      Author: Nicolas Turro <nicolas.turro.at.inria.fr>
 */

#include "platform.h"
#include "iotlab-a8-m3/iotlab-a8-m3.h"
#include "printf.h"
#include "gpio.h"
#include "afio.h"
#include "nvic_.h"

// our STM32F103 runs at 72Mhz, 72000065 is an average measurement from a external GPS PPS source.
#define CYCLESPERSEC 72000065.0
#define CYCLES2NS 1000000000.0/CYCLESPERSEC

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
  (void)arg;
  // we recieve this interrupt each second from the GPS PPS
  // so we increment the number of seconds elapsed from the beginning
  seconds++;
  // and we reset the cycle counter
  *DWT_CYCCNT = 0;
}

void char_rx(handler_arg_t arg, uint8_t c)  {
  static union {
          uint8_t c[4];
          uint32_t i;
  } A8buf;
  static int A8comCount;

  asm volatile ("cpsid i");

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
    A8buf.c[A8comCount]=c;
    A8comCount++;
    if (A8comCount==4) {
      seconds = A8buf.i;
      A8comState=TIMEOK;
    }
    break;
  case TIMEOK :
    if (c==MAGICCHAR1) A8comState=MAGIC1;
    break;
  }
  asm volatile ("cpsie i");
}

int main() {
  time_t curtime;
  int i;
  platform_init();

  printf("nico test\n===================\n");

  uart_set_rx_handler(uart_print, char_rx, NULL);

  while(A8comState!=TIMEOK){
    asm volatile ("wfi");
  }

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
  exti_set_handler(EXTI_LINE_Px3, PPShandler, NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px3, EXTI_TRIGGER_RISING);

  while(1){
    //wait a bit
    for(i=0;i<1000000;i++)    asm("nop");
    //get and print the elapsed time
    getTime(&curtime);
    printf("Time : %d.%09d\n",curtime.sec,curtime.nsec);
  };
  return 0;
}
