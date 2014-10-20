#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "constants.h"

/*
 * Frame definition
 */
#define FRAME_LENGTH_MAX 33
#define POLL_LENGTH_MAX 255
#define POLLING_MEASURE_MAX 61 //on 16bits was 123 but float for power
#define POWER_POLLING_MEASURE_MAX 50
#define RADIO_POLLING_MEASURE_MAX 21 //21 * 6 = 126
#define RADIO_POLLING_TIME_MAX 50 //ms
#define RADIO_POLLING_TIME_MAX_US ((double) (RADIO_POLLING_TIME_MAX * 1000)
#define POWER_POLLING_TIME_MAX 50
#define NBR_ERROR_FRAME 5

/* Typedefs */
typedef union {
    float nbr;
    uint8_t byte[4];
} float2byte_t;

typedef union {
    uint32_t nbr;
    uint8_t byte[4];
} uint32_t2byte_t;

typedef union {
    struct {
        uint8_t sync;
        uint8_t len;
        uint8_t type;
        uint8_t payload[FRAME_LENGTH_MAX - 1];
    };
    uint8_t data[FRAME_LENGTH_MAX + 2];
} xRXFrame_t;

typedef union {
    struct {
        uint8_t sync;
        uint8_t len;
        uint8_t type;
        uint8_t ack;
        uint8_t payload[FRAME_LENGTH_MAX - 2];
    };
    uint8_t data[FRAME_LENGTH_MAX + 2];
} xTXFrame_t;

typedef union {
    struct {
        uint8_t sync, len, type, count,
        //measures[4 * POWER_POLLING_MEASURE_MAX][4];
        measures[4 * 4 * POWER_POLLING_MEASURE_MAX];
    };
    //uint8_t data[2 * POLLING_MEASURE_MAX + 13];
    //header=4bytes, max (4 meas of 4 bytes) POWER_POLLING_MEASURE_MAX time
    uint8_t data[4 + 4 * 4 * POWER_POLLING_MEASURE_MAX];
} xPowerFrame_t;

typedef struct
{
    uint32_t2byte_t time;
    int8_t rssi;
    uint8_t lqi;
} __attribute__ ((__packed__)) xRadioStat_t;

// considering some reception every 1ms and send to be done each 50ms
// considering buffer for 50 measures, 4B header + 50*6 = 304 bytes
// take care about struct alignment and its side effect
#define RADIO_POLLING_STAT_MAX 50
typedef struct {
    union {
        struct {
            //uint8_t sync, len, type, conf, count;
            uint8_t sync, len, type, count;
            xRadioStat_t stat[RADIO_POLLING_STAT_MAX];
        }__attribute__ ((__packed__));
        //uint8_t data[5 + 6 * RADIO_POLLING_STAT_MAX];
        //header=4bytes, 1 byte for RSSI and 1 for LQI
        uint8_t data[4 + 6 * RADIO_POLLING_STAT_MAX];
    };
} xRadioFrame_t;

//Acknowledge Config Changed Frame
//config2 only used for radio ack frame
typedef struct {
    union {
        struct {
            uint8_t sync, len, type, config_typ, config, config2;
        };
        uint8_t data[6];
    };
} xAckConfigChanged_t;

/*
 * Error frame
 * define in global a table of this frame, used indicate frame in preparation
 */
typedef struct {
    union {
        struct {
            uint8_t sync, len, type, error;
        };
        uint8_t data[4];
    };
    uint8_t used;
} xErrorFrame_t;

xSemaphoreHandle xErrorFrameMutex[5];
// global variable set when no free frame in xErrorFrame[nbrErrorFrame]
// and defensive issue during error frame treatment
uint8_t no_frame_error_available_defensive_issue;
xErrorFrame_t xErrorFrame[NBR_ERROR_FRAME];



extern void* uart1_rx_cb;

/* time reference */
extern uint32_t time0;


#endif
