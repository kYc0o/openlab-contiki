#ifndef GPS_SYNCED_CLOCK_H
#define GPS_SYNCED_CLOCK_H

typedef struct 
{
  uint32_t  s;
  uint16_t  ms;
  uint16_t  us;

} gps_synced_time_t;

// Configure PPS + GPIO + interrupt handler
void gps_synced_clock_init(void);

// Return the value of the clock
void gps_synced_clock_get(gps_synced_time_t* result);

#endif
