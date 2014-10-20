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

//  DWT_CYCNT counter is used instead of SysTick because it's a 32bits counter which overflows once a minute whereas
// SysTick overflows every 234 ms
volatile unsigned int *DWT_CYCCNT     = (volatile unsigned int *)0xE0001004; //address of the register
volatile unsigned int *DWT_CONTROL    = (volatile unsigned int *)0xE0001000; //address of the register
volatile unsigned int *SCB_DEMCR         = (volatile unsigned int *)0xE000EDFC; //address of the register

volatile uint32_t seconds=0;

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
  static uint32_t cycles;
  // we recieve this interrupt each second from the GPS PPS
  // so we increment the number of seconds elapsed from the beginning
  cycles=*DWT_CYCCNT;
  *DWT_CYCCNT = 0;
  seconds++;
  // and we reset the cycle counter
  printf("%u\n",cycles);
}

int main() {
  int i;
  platform_init();

  printf("nico test\n===================\n");

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
    for(i=0; i < 1000000; i++)
            asm("wfi");
    //get and print the elapsed time
    //getTime(&curtime);
    //    printf("Time : %d.%09d\n",curtime.sec,curtime.nsec);
  };
  return 0;
}
