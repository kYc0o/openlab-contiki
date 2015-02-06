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
#include "gpio.h"
#include "printf.h"
#include "event.h"
#include "soft_timer.h"

#include "synchro_out.h"

// timer alarm function
static void alarm(handler_arg_t arg);
static soft_timer_t tx_timer;
#define BLINK_PERIOD soft_timer_s_to_ticks(1)

static void button_pressed();

// Function to manage GPIO (global variable in synchro.h)
inline void pin_enable_out(uint8_t i)
{
  gpio_enable(gpio_config[i].port);
  gpio_set_output(gpio_config[i].port, gpio_config[i].pin);
}

inline void pin_clear(uint8_t i)
{
  gpio_pin_clear(gpio_config[i].port, gpio_config[i].pin);
}

inline void pin_set(uint8_t i)
{
  gpio_pin_set(gpio_config[i].port, gpio_config[i].pin);
}

static void hardware_init()
{
 // Openlab platform init
  platform_init();
  event_init();
  soft_timer_init();
  
  // Switch off the LEDs
  leds_off(LED_0 | LED_1 );
  // Enable and set output GPIO 0,1,2 of the Fox daughter board
  pin_enable_out(0);
  pin_enable_out(1);
  pin_enable_out(2);
  // Clear pin 0,1,2
  pin_clear(0);
  pin_clear(1);
  pin_clear(2);
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

// Manage the Fox button
static void button_pressed()
{ 
  // switch on the LED and Set pins 0,1,2
  leds_on(LED_0);
  pin_set(0);
  pin_set(1);
  pin_set(2);
  // Wait 2 seconds
  soft_timer_delay_s(2);
  // switch off the LED and Clear pins 0,1,2
  leds_off(LED_0);
  pin_clear(0);
  pin_clear(1);
  pin_clear(2);
}

// Blinking LED
static void alarm(handler_arg_t arg) {
  leds_toggle(LED_1);
}
