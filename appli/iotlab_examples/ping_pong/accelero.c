#include <platform.h>
#include <event.h>
#include <lsm303dlhc.h>

#include "accelero.h"


static void handle_interrupt(handler_arg_t);
static void handle_event(handler_arg_t);
static void bootstrap_interrupts();

static void (*event_handler)(uint32_t);

void accelero_init(void (*callback)(uint32_t))
{
	event_handler = callback;
	lsm303dlhc_powerdown();
	lsm303dlhc_acc_config(
		LSM303DLHC_ACC_RATE_100HZ,	// ACC_RATE_400HZ,
		LSM303DLHC_ACC_SCALE_2G,	// ACC_SCALE_16G,
		LSM303DLHC_ACC_UPDATE_ON_READ);	// UPDATE_CONTINUOUS
	lsm303dlhc_acc_set_drdy_int1(handle_interrupt, NULL);
	bootstrap_interrupts();
}

static void bootstrap_interrupts()
{
	int16_t a[3];
	lsm303dlhc_read_acc(a);
}

static void handle_event(handler_arg_t ignored)
{
	int16_t a[3] = { 0, 0 ,0 };
	while (lsm303dlhc_acc_get_drdy_int1_pin_value())
		lsm303dlhc_read_acc(a);

	uint32_t accel = a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
	if (accel > 800000)
		return;

	event_post(
		EVENT_QUEUE_APPLI,
		(handler_t)event_handler,
		(handler_arg_t)accel);
}

static void handle_interrupt(handler_arg_t ignored)
{
	event_post_from_isr(EVENT_QUEUE_APPLI, handle_event, NULL);
}
