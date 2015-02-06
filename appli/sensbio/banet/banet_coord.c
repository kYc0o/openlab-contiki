/*
* This file is a part of openlab/sensbiotk
*
* Copyright (C) 2015  INRIA (Contact: sensbiotk@inria.fr)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * banet_coord.c
 *
 * \brief banet TDMA coordinator
 *
 * \date Jan 06, 2015
 * \author: <roger.pissard.at.inria.fr>
 */

#include "platform.h"
#include "packet.h"
#include "soft_timer.h"

#include "mac_tdma.h"

#include "debug.h"

/*
 * coordinator configuration:
 * + network ID 0x6666
 * + channel 21
 * + 4 slots (=> 2 client nodes) of 10ms each
 */
static mac_tdma_coord_config_t cfg = {
    /* network id */
    .panid = 0x6666,
    /* phy channel */
    .channel = 21,
    /* slot duration in tdma time unit (default 1unit = 100us) */
    .slot_duration = 100,
    /* number of slots (the coordinator can handle up to count-2 nodes) */
    .slot_count = 4,
};

static void pkt_received(packet_t *packet, uint16_t src);

int main()
{
    platform_init();

    /* init tdma */
    mac_tdma_init();

    /* start coordinator */
    mac_tdma_start_coord(&cfg);

    /* register data packet handler */
    mac_tdma_set_recv_handler(pkt_received);

    /* shutdown leds */
    leds_off(0xf);

    platform_run();
    return 0;
}


static void pkt_received(packet_t *packet, uint16_t src)
{
    // unused
    (void) src;

    if (packet->length == 1)
    {
        log_printf("Packet received from 0x%04x : %u\n", src, *(packet->data));
    }
    else
    {
        log_printf("Unknown Packet received from 0x%04x\n", src);
    }
    packet_free(packet);
}
