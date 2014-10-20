#include "platform.h"
#include "printf.h"
#include <stdlib.h> // atoi
#include "soft_timer.h"

/*
	Flood the serial link with printfs of ints output at constant rate.

	The rate of the flood is specified by parameter "grace_time_ms".
	To configure grace_time_ms without re-compiling, use:
	
		sed -i 's/0005/xxxx/' ../../../bin/test_serial_flood.elf
*/

static char *grace_time_ms = "0005";

static int k = 0;
void do_serial_out() {
	printf("%d\n", ++k);
}

static soft_timer_t timer;
#define SOFT_TIMER_PERIODIC 1

int main()
{
	int period = atoi(grace_time_ms);
	int ticks = soft_timer_ms_to_ticks(period ? period : 5);

	platform_init();
	soft_timer_init();
	soft_timer_set_handler(&timer, (handler_t)do_serial_out, NULL);
	soft_timer_start(&timer, ticks, SOFT_TIMER_PERIODIC);
	platform_run();

	return 0;
}
