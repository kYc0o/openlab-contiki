#include "printf.h"
#include "time.h"

uint32_t get_seconds(uint32_t timer_tick)
{
        return timer_tick / TIMER_FACTOR;
}

uint32_t get_microseconds(uint32_t timer_tick)
{
        uint32_t aux_32;

        /* normal calculation with a 64b
         * uint64_t aux;
         * aux  = (timer_tick % TIMER_FACTOR);
         * aux *= 1000000; // convert to useconds before dividing to keep as integer
         * aux /= TIMER_FACTOR;
         */

        // hack taking into account that 2^6 is Greatest Common Divisor of
        // one million and TIMER_FACTOR,
        // and that (TIMER_FACTOR * 1000000 / GCD) <= 2**32 -1
        aux_32  = (timer_tick % TIMER_FACTOR);
        aux_32 *= (1000000      / (1 << 6));
        aux_32 /= (TIMER_FACTOR / (1 << 6));

        return aux_32;
}


/*
 * Get the absolute time using a 'ticks' timer, and a 'seconds' timer
 *
 * The 'seconds' timer should be have a value taken after the 'ticks' timer.
 * The function supports timer_secs to be taken 0xF seconds after measuring timer_tick.
 *
 */

void get_absolute_time(struct timeval *absolute_time, \
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

        // Works because 2**32 is a multiple of TIMER_FACTOR
        // Get the big part from the seconds, and the small from the ticks.
        final_secs = (seconds_s & (~ 0xF)) | (seconds_ticks & (0xF));
        // remove the 'overflow' if necessary
        final_secs -= (seconds_s & (0x10)) ^ (seconds_ticks & (0x10));

        absolute_time->tv_sec = final_secs;
        absolute_time->tv_usec = get_microseconds(timer_tick);
}

