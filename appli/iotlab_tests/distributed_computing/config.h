#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include "printf.h"
#include "iotlab_uid.h"
#include "phy.h"


// choose channel in [11-26]
#define CHANNEL 12

#define NUM_VALUES           2
#define MIN_RSSI           -70


#define GRAPH_RADIO_POWER PHY_POWER_m17dBm
#define RADIO_POWER PHY_POWER_3dBm

#define MAC_PKT_LEN (PHY_MAX_TX_LENGTH - 4)
#define MAX_NUM_NEIGHBOURS  ((MAC_PKT_LEN -2)/ sizeof(uint16_t))


#define MSG(fmt, ...) printf("%04x;" fmt, iotlab_uid(), ##__VA_ARGS__)

#define ERROR(fmt, ...) printf("ERROR:" fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) printf("INFO:" fmt, ##__VA_ARGS__)
#if 0
#define DEBUG(fmt, ...) printf("DEBUG:" fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif



#endif//CONFIG_H
