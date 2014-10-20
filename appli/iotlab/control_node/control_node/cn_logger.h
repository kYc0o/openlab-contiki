#ifndef CN_LOGGER_H
#define CN_LOGGER_H

#include <string.h>
#include "printf.h"
#include "packet.h"
#include "constants.h"

extern packet_t *cn_logger_pkt;

/** Alloc the cn_logger internal pkt */
void cn_logger_reset();

/** Send logger message */
#define cn_logger(level, msg, args...) do {                                    \
                                                                               \
    cn_logger_reset();                                                         \
    packet_t *pkt = cn_logger_pkt;                                             \
    if (pkt == NULL)                                                           \
        break;                                                                 \
                                                                               \
    pkt->data[0] = (level);                                                    \
    snprintf((char *)&pkt->data[1], IOTLAB_SERIAL_DATA_MAX_SIZE - 1,           \
            (msg) , ##args);                                                   \
    pkt->length = 2 + strlen((char *)&pkt->data[1]);                           \
                                                                               \
    if (0 == iotlab_serial_send_frame(LOGGER_FRAME, pkt))                      \
        cn_logger_pkt = NULL;  /* Success */                                   \
                                                                               \
    cn_logger_reset();                                                         \
                                                                               \
} while (0);


#endif // CN_LOGGER_H
