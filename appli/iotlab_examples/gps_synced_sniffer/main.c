#include <platform.h>
#include <printf.h>

#include "phy.h"
#include "event.h"

#include "zep_sniffer.h"
#include "gps_synced_clock.h"

// channel between 11 and 26
static uint8_t channel = 11;

static void hardware_init()
{
    platform_init();
    event_init();
    soft_timer_init();
    gps_synced_clock_init();
    leds_off(0xFF);
}

int main()
{
    hardware_init();
    init_sniffer(channel);
    platform_run();
    return 0;
}
