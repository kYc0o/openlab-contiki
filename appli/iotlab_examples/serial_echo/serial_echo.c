/*
 * echo serial
 *
 *  Created on: Dec 13, 2012
 *      	harter
 */

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "platform.h"

#include "printf.h"
#include "soft_timer.h"
#include "soft_timer_delay.h"

#include "queue.h"

static void app_task(void *);

static void char_rx(handler_arg_t arg, uint8_t c);

static unsigned int buff_index = 0;
static char buff[1024] = {'\0'};


static xQueueHandle char_queue = 0; // set to 0 to prevent crashing on startup
static void char_rx(handler_arg_t arg, uint8_t c)
{
	// ok if char_queue is not created
	xQueueSendFromISR(char_queue, &c, 0);
}


void echo_line()
{
	char value;
	while (xQueueReceive(char_queue, &value, 0) == pdTRUE ) {
		if (value == '\r') {
			; // manage it with \n
		} else if (value == '\n') {
			buff[buff_index++] = '\n';
			buff[buff_index] = '\0';
			printf(buff);

			buff_index = 0;
		} else {
			// 1023 strict to get enough space for '\n' and
                        // '\0' at the end
			if (buff_index < 1022) {
				buff[buff_index++] = value;
			}
		}
	}
}


int main()
{
	// Initialize the platform
	platform_init();


	uart_set_rx_handler(uart_print, char_rx, NULL);
	/* Group all the FreeRTOS calls after hardware initialisation
	 * because FreeRTOS disables interrupts before platform_run */

	// 8 elements: Parce que 8, ça suffit !!! (cf. Reflets d'acide)
	char_queue = xQueueCreate(8, sizeof(char));

	xTaskCreate(app_task, (const signed char * const) "app",
			configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	// Run
	platform_run();
	return 0;
}


static void app_task(void *param)
{
	printf("STARTING\n");

	while (1) {
		echo_line();
	}
}



