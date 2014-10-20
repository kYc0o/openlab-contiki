#ifndef LIB_COMM_H_
#define LIB_COMM_H_

#include <stdint.h>

#include "omlcore.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FRAME_LENGTH_MAX 512
#define SYNC 0X80

typedef enum {
  INIT = 0x1,
  ADD_MP = 0x2,
  START = 0x3,
  INJECT = 0x4,
  INJECT_METADATA = 0x5,
  PROCESS = 0x6,
  CLOSE = 0x7
} CommandT;

typedef union {
  struct {
    uint8_t sync;
    uint8_t len;
    uint8_t type;
    uint8_t payload[FRAME_LENGTH_MAX - 2];
  };
  uint8_t data[FRAME_LENGTH_MAX + 2];
} FrameT;

#define cmd_init(c, t) \
  do {                 \
    c.sync = SYNC;     \
    c.len = 1;         \
    c.type = t;        \
  } while (0)

#define cmd_send(c) \
  uart_transfer(uart_print, c.data, c.len + 2)

#define send_type(type) \
  do {                             \
    FrameT cmd;                 \
    cmd_init(cmd, type);           \
    cmd_send(cmd);                 \
  } while(0)

void write_bytes(FrameT *cmd, const void *source, uint8_t len);
void write_byte(FrameT *cmd, uint8_t n);
void write_string(FrameT *cmd, const char* str);
void write_mp(FrameT *cmd, const OmlMP *mp);
void write_mpdef(FrameT *cmd, const OmlMPDef *mpdef, uint8_t param_count);
void write_value(FrameT *cmd, OmlValueT type, const OmlValueU *value);
void write_values(FrameT *cmd, OmlMP *mp, const OmlValueU *values);

#ifdef __cplusplus
}
#endif

#endif /*LIB_COMM_H_*/
