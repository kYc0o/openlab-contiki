#include "soft_timer_delay.h"
#include "iotlab_time.h"


static inline uint32_t get_microseconds(uint32_t timer_tick);
static inline uint32_t get_seconds(uint32_t timer_tick);
static void get_absolute_time(struct soft_timer_timeval *absolute_time,
        uint32_t timer_tick, uint32_t timer_secs);


struct soft_timer_timeval time0 = {0, 0};
struct soft_timer_timeval unix_time_ref = {0, 0};

void iotlab_time_set_time(struct soft_timer_timeval *t0,
        struct soft_timer_timeval *time_ref)
{
    time0 = *t0;
    unix_time_ref = *time_ref;
}

void iotlab_time_extend_relative(struct soft_timer_timeval *time,
        uint32_t timer_tick)
{
    /* extend timer_tick to timeval */
    *time = soft_timer_time_extended();
    get_absolute_time(time, timer_tick, time->tv_sec);

    /* Set time relative to time0 */
    time->tv_sec -= time0.tv_sec;
    if (time->tv_usec < time0.tv_usec) {
        time->tv_usec += 1000000;
        time->tv_sec  -= 1;
    }
    time->tv_usec -= time0.tv_usec;

    /* Add unix time */
    time->tv_sec  += unix_time_ref.tv_sec;
    time->tv_usec += unix_time_ref.tv_usec;

    /* Correct usecs > 100000 */
    if (time->tv_usec > 1000000) {
        time->tv_sec  += 1;
        time->tv_usec -= 1000000;
    }
}



/*
 * Get the absolute time using a 'ticks' timer, and a 'seconds' timer
 *
 * The 'seconds' timer should be have a value taken after the 'ticks' timer.
 * The function supports timer_secs to be taken 0xF seconds after measuring timer_tick.
 *
 */
static void get_absolute_time(struct soft_timer_timeval *absolute_time,
        uint32_t timer_tick, uint32_t timer_secs)
{
    uint32_t seconds_s     = timer_secs;
    uint32_t seconds_ticks = get_seconds(timer_tick);

    uint32_t final_secs;


    /*
     *
     * Get the small variations from the timer_ticks calculation of seconds
     * And the big part of the value from the timer_seconds
     *
     * In the case where the fact that timer_seconds is taken later,
     * makes its 4 LSB bytes "overflow", 0x10 should be removed
     * It's detected comparing the 5th LSB of both timers calculation of seconds.
     *
     * Example:
     * seconds_ticks = 0x0F0A
     * seconds_s = 0x0F0C (taken 2 seconds later)
     * final_secs = 0x0F00 | 0x000A == 0x0F00A
     * final_secs -= 0  // everything fine
     * final_secs == 0x0F0A == seconds_ticks (which is right for small values)
     *
     * seconds_ticks = 0x0FFE
     * seconds_s = 0x1000 (taken 2 seconds later) // here, 0x10 seconds should be removed
     * final_secs = 0x1000 | 0x000E == 0x100E
     * final_secs -= 0x10
     * final_secs = 0x0FFE == seconds_ticks (whics is what is expected for small values)
     *
     */

    // Works because 2**32 is a multiple of 32768
    // Get the big part from the seconds, and the small from the ticks.
    final_secs = (seconds_s & (~ 0xF)) | (seconds_ticks & (0xF));
    // remove the 'overflow' if necessary
    final_secs -= (seconds_s & (0x10)) ^ (seconds_ticks & (0x10));

    absolute_time->tv_sec = final_secs;
    absolute_time->tv_usec = get_microseconds(timer_tick);
}



static inline uint32_t get_seconds(uint32_t timer_tick)
{
    return timer_tick / SOFT_TIMER_FREQUENCY;
}

static inline uint32_t get_microseconds(uint32_t timer_tick)
{
    /*
     * 2^6 is Greatest Common Divisor of one million and 32768,
     * and (32768 * 1000000 / GCD) <= 2**32 -1
     */

    uint32_t aux_32;
    aux_32  = (timer_tick % SOFT_TIMER_FREQUENCY);
    aux_32 *= (1000000 / (1 << 6));
    aux_32 /= (SOFT_TIMER_FREQUENCY / (1 << 6));
    return aux_32;

    /* normal calculation with a 64b
     * uint64_t aux;
     * aux  = (timer_tick % SOFT_TIMER_FREQUENCY;
     * aux *= 1000000; // convert to useconds before dividing to keep as integer
     * aux /= SOFT_TIMER_FREQUENCY;
     */
}

