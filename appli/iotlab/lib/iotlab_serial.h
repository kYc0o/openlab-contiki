/*
 * This file is part of HiKoB Openlab.
 *
 * HiKoB Openlab is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, version 3.
 *
 * HiKoB Openlab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HiKoB Openlab. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2013 HiKoB.
 */

/*
 * iotlab-serial.h
 *
 *  Created on: Aug 12, 2013
 *      Author: burindes
 *              harterg
 */

#ifndef IOTLAB_SERIAL_H
#define IOTLAB_SERIAL_H

#include <stddef.h>
#include <stdint.h>
#include "packet.h"

enum {
    /** Offset to use when allocating a packet (for header) */
    IOTLAB_SERIAL_HEADER_SIZE = 3,
    /*
     * Maximum payload size
     * Minimum between 254: (WCHAR_MAX - 1) (1 for pkt type)
     * and size storable in only one packet
     */
    _PAYLOAD_MAX = (PACKET_MAX_SIZE - IOTLAB_SERIAL_HEADER_SIZE),
    IOTLAB_SERIAL_DATA_MAX_SIZE = (_PAYLOAD_MAX < 254 ? _PAYLOAD_MAX : 254),
};

/** Start the serial library, at the specified baudrate */
void iotlab_serial_start(uint32_t baudrate);

/** Set the packet type for log packets */
void iotlab_serial_set_logger_byte(uint8_t logger_type_byte);

typedef struct {
    /** The command type for which the handler should be called */
    uint8_t cmd_type;
    /**
     * The handler function called on each command received with matching type.
     *
     * The handler receives a packet with data contained in it, and should
     * update the same packet with the response.
     *
     * The handler should return 1 if command processed successfully or 0 if an
     * error occurred.
     */
    int32_t (*handler)(uint8_t cmd_type, packet_t *pkt);

    /** Next pointer to chain list */
    void* next;
} iotlab_serial_handler_t;

packet_t *iotlab_serial_packet_alloc(void);

/**
 * Register a serial handler.
 * This method registers a handler for a command type based on the structure.
 *
 * The provided structure will be chained internally, and data must be persistent
 * at all time.
 *
 * \param handler a pointer to the structure containing the information.
 */
void iotlab_serial_register_handler(iotlab_serial_handler_t *handler);

/**
 * Send an asynchronous frame.
 *
 * \param type the frame type
 * \param pkt a pointer to the packet to send. It will be freed if sent successfully.
 * \return 0 if packet sent OK, 1 if an error occurred.
 */
int32_t iotlab_serial_send_frame(uint8_t type, packet_t *pkt);


/**
 * Append data in the serial packet
 */
void iotlab_serial_append_data(packet_t *pkt, void *data, size_t size);


#endif /* IOTLAB_SERIAL_H_*/
