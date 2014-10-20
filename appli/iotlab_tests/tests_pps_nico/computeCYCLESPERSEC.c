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
#include "soft_timer_delay.h"
#include "afio.h"
#include "nvic_.h"

// our STM32F103 runs at 72Mhz, 72000065 is an average measurement from a external GPS PPS source. 
#define CYCLESPERSEC 72000065

volatile unsigned int *DWT_CYCCNT     = (volatile unsigned int *)0xE0001004; //address of the register
volatile unsigned int *DWT_CONTROL    = (volatile unsigned int *)0xE0001000; //address of the register
volatile unsigned int *SCB_DEMCR         = (volatile unsigned int *)0xE000EDFC; //address of the register
int prevcyc;
int sumcyc=0;
int count=0;

void PPShandler(void) {
  int curcyc;
  curcyc=*DWT_CYCCNT;
  sumcyc=sumcyc+(curcyc-prevcyc)-72000000;
  prevcyc=curcyc;
  count++;
  if (count==10) {
    printf("Average number of cycles between two PPS : %d\n",72000000+(sumcyc/count));
    count=0;sumcyc=0;
  }
}

int main()
{
  uint32_t PPS_status;
  uint32_t PPS_new_status;
  uint32_t t_a, t_b;

  platform_init();
  printf("nico test\n===================\n");
  
  *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
  *DWT_CYCCNT = 0; // reset the counter
  *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter

  prevcyc=*DWT_CYCCNT;
  // GPS PPS output is wired to GPIO PA3
  gpio_set_input(gpioA, GPIO_PIN_3);
  gpio_enable(gpioA);

  // link GPIO PA3 to IRQ_LINE_EXTI3
  afio_select_exti_pin(EXTI_LINE_Px3, AFIO_PORT_A);
  nvic_enable_interrupt_line(NVIC_IRQ_LINE_EXTI3);

  // set IRQ handler and configure call to RISING signal on PA3
  exti_set_handler(EXTI_LINE_Px3, PPShandler,NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px3, EXTI_TRIGGER_RISING);



  PPS_status=gpio_pin_read(gpioA,GPIO_PIN_3);

  t_a=soft_timer_time();

  while(1){
    asm("nop");
    /*    PPS_new_status=gpio_pin_read(gpioA,  GPIO_PIN_3);
    if (PPS_new_status !=PPS_status) {
      PPS_status=PPS_new_status;
      t_b = soft_timer_time();
      if (PPS_new_status > 0) {
	printf("got PPS pulse after %d ticks\n",t_b - t_a);
      } else {
	printf("end of PPS pulse after %d ticks\n", t_b - t_a);
      }
      t_a=t_b;
    }
    */
  };
  return 0;
}
