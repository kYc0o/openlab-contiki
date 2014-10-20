#ifndef _CONTROL_NODE_H_
#define _CONTROL_NODE_H__

/*
 * Send the frame passed in parameter to the UART
 * \param * data is the pointer to beginning of the frame
 * \param len is the length of byte of the frame to be sent
 * The UART access is protected by a mutex
 */
void vSendFrame(uint8_t* data, int16_t len);

/*
 * Wrapper to Send an Error Frame by calling vSendFrame
 * There is a table of Error Frames. Several error frames could occurs at the
 * same time.
 * A mutex is used to ensure atomic release of this specific error frame, once
 * sent.
 * \frame_nbr is the index of the Error Frame to send
 */
void vSendErrorFrame(uint8_t frame_nbr);

#endif
