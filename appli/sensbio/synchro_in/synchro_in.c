/*
* This file is a part of openlab/sensbiotk
*
* Copyright (C) 2015  INRIA (Contact: sensbiotk@inria.fr)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "platform.h"
#include "agilefox.h"
#include "stm32f1xx.h"
#include "gpio.h"
#include "exti.h"
#include "afio.h"
#include "nvic_.h"
#include "printf.h"
#include "event.h"
#include "soft_timer.h"

// timer alarm function
static void alarm(handler_arg_t arg);
static soft_timer_t tx_timer;
#define BLINK_PERIOD soft_timer_s_to_ticks(1)
// button handler
static void button_pressed();
// Input Synchronization handler
static void sync_handler(handler_arg_t arg);

static void hardware_init()
{
 // Openlab platform init
  platform_init();
  event_init();
  soft_timer_init();
  
  // Switch off the LEDs
  leds_off(LED_0 | LED_1 );
  // Enable and set input GPIO 0 of the Fox daughter board
  gpio_set_input(GPIO_A, GPIO_PIN_7);
  gpio_enable(GPIO_A);
  // link GPIO to IRQ line 
  afio_select_exti_pin(EXTI_LINE_Px7, AFIO_PORT_A);
  nvic_enable_interrupt_line(NVIC_IRQ_LINE_EXTI9_5);
  // set IRQ handler and configure call to RISING signal
  exti_set_handler(EXTI_LINE_Px7, sync_handler, NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px7, EXTI_TRIGGER_RISING);
  // callback connection to the Fox button 
  button_set_handler(button_pressed, NULL);
  // Initialize a openlab timer for blinking LED
  soft_timer_set_handler(&tx_timer, alarm, NULL);
  soft_timer_start(&tx_timer, BLINK_PERIOD, 1);
}

// Launch the application
int main()
{
  hardware_init();
  platform_run();
  return 0;
}

// Manage the Synch Input
static void sync_handler(handler_arg_t arg) {
  // switch on the LED and Set pins 0,1,2
  leds_on(LED_0);
  // Wait 1 seconds
  soft_timer_delay_s(1);
  // switch off the LED and Clear pins 0,1,2
  leds_off(LED_0);
}

// Manage the Fox button
static void button_pressed()
{ 
  // switch on the LED and Set pins 0,1,2
  leds_on(LED_0);
  // Wait 1 seconds
  soft_timer_delay_s(1);
  // switch off the LED and Clear pins 0,1,2
  leds_off(LED_0);
}

// Blinking LED
static void alarm(handler_arg_t arg) {
  leds_toggle(LED_1);
}
