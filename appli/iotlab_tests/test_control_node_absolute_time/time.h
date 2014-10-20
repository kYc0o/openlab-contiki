#ifndef TIME_H
#define TIME_H
#include <stdint.h>


enum constants {
        TIMER_FACTOR = 32768,
};
struct timeval {
        uint32_t  tv_sec;     /* seconds */
        uint32_t  tv_usec;    /* microseconds */
};


void get_absolute_time(struct timeval *absolute_time, \
                       uint32_t timer_tick, uint32_t timer_secs);

#endif // TIME_H
