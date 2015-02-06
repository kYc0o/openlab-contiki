#include <platform.h>
#include <printf.h>
#include <time.h>

#include "gps_synced_clock.h"

#include "stm32f1xx.h"
#include "gpio.h"
#include "afio.h"
#include "nvic_.h"

#include "iotlab_i2c.h"

static volatile uint32_t seconds;

void gps_synced_clock_get(gps_synced_time_t* result)
{
  uint32_t tmp;

  vPortEnterCritical();

  result->s = seconds;
  result->us = timer_time(TIM_2);
  result->ms = timer_time(TIM_4);
  tmp =  timer_time(TIM_2);

  if (tmp < result->us)
  {
    result->us = tmp;
    result->ms++; 
  }

  if (result->ms > 1000)
    result->ms = 1000;

  vPortExitCritical();
}

static void PPS_handler(handler_arg_t args)
{
  vPortEnterCritical();

  seconds++;

  *timer_get_CNT(TIM_2) = 0;
  *timer_get_CNT(TIM_4) = 0;

  vPortExitCritical();
}

void configure_feeding_timer(openlab_timer_t timer)
{
  timer_select_internal_clock(timer,
      (rcc_sysclk_get_clock_frequency(RCC_SYSCLK_CLOCK_PCLK1_TIM)/1000000)-1);

  timer_start(timer, 1000, NULL, NULL);

  // config TRGO
  *timer_get_SMCR(timer) |= TIMER_SMCR__MSM;
  #define TIM_TRGOSource_Update              ((uint16_t)0x0020)
  *timer_get_CR2(timer) |= TIM_TRGOSource_Update;
}

void configure_cascaded_timer(openlab_timer_t timer)
{
  const _openlab_timer_t *_timer = timer;

  *timer_get_SMCR(_timer) = TIMER_SMCR__SMS_EXTERNAL_CLOCK_MODE_1;

  // Input trigger
  uint16_t tmpsmcr = 0;
  tmpsmcr = *timer_get_SMCR(_timer);
  tmpsmcr &= (uint16_t)(~((uint16_t)TIMER_SMCR__TS_MASK));
  #define TIM_TS_ITR1      ((uint16_t)0x0010)
  tmpsmcr |= TIM_TS_ITR1;
  *timer_get_SMCR(_timer) = tmpsmcr;

  // Enable CR1 ARPE
  *timer_get_CR1(timer) |= TIMER_CR1__ARPE;

  // Enable the counter
  *timer_get_CR1(_timer) |= TIMER_CR1__CEN;
}

static void get_control_node_unix_time()
{
  struct soft_timer_timeval time;
  seconds = 0;
  iotlab_i2c_init();

  printf("Getting control_node time\n");
  iotlab_get_time(&time);

  // Sleep until next half second
  uint32_t sleeptime = 1500000 - time.tv_usec;
  soft_timer_delay_us(sleeptime);

  iotlab_get_time(&time);
  seconds = time.tv_sec;

  struct tm *local_time = gmtime((time_t *)&time.tv_sec);
  char time_str[64];
  strftime(time_str, (sizeof time_str), "%Y-%m-%d %H:%M:%S", local_time);
  printf("Using Epoch: %u UTC %s\n", time.tv_sec, time_str);
}


void gps_synced_clock_init(void)
{
  seconds = 0;

  // GPS PPS output is wired to GPIO PA3
  gpio_set_input(gpioA, GPIO_PIN_3);
  gpio_enable(gpioA);

  // link GPIO PA3 to IRQ_LINE_EXTI3
  afio_select_exti_pin(EXTI_LINE_Px3, AFIO_PORT_A);
  nvic_enable_interrupt_line(NVIC_IRQ_LINE_EXTI3);

  // set IRQ handler and configure call to RISING signal on PA3
  exti_set_handler(EXTI_LINE_Px3, PPS_handler, NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px3, EXTI_TRIGGER_RISING);

  // run TIM2 @ 1MHz
  timer_enable(TIM_2);
  configure_feeding_timer(TIM_2);
  // TIM4 is clocked by TIM2 overflow signal.
  timer_enable(TIM_4);
  configure_cascaded_timer(TIM_4);

  // Get time from the control node
  get_control_node_unix_time();
}
