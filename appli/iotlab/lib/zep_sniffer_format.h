#ifndef ZEP_SNIFFER_FORMAT_H
#define ZEP_SNIFFER_FORMAT_H

#include <stdint.h>
#include "phy.h"

/* Encode pkt to zep format and save it in dst
 * dst should be at min 32 + 2 + packet length */
size_t to_zep(uint8_t *dst, phy_packet_t *pkt, uint8_t channel,
        uint16_t device_id);


#endif /* ZEP_SNIFFER_FORMAT_H */
