#include <stdlib.h>
#include <stdint.h>

#include "omlc.h"
#include "comm.h"
#include "omlmp.h"

#include "uart_.h"

//extern uart_t uart_print;


int8_t omlc_init(const char *appName) {
  FrameT cmd;

  uart_enable(uart_print, 500000);

  cmd_init(cmd, INIT);
  write_string(&cmd, appName);
  cmd_send(cmd);
  
  return 0;
}

OmlMP *omlc_add_mp(const char *mp_name, OmlMPDef *mp_def) {
  FrameT cmd;
  OmlMP *mp;

  mp = _OmlMP_create(mp_name, mp_def);
  
  cmd_init(cmd, ADD_MP);
  write_string(&cmd, mp_name);
  write_mpdef(&cmd, mp_def, mp->param_count);

  cmd_send(cmd);
  
  return mp;
}

int8_t omlc_start(void) {
  send_type(START);
  return 0;
}

int8_t omlc_inject(OmlMP *mp, OmlValueU *values) {
  FrameT cmd;

  cmd_init(cmd, INJECT);
  write_mp(&cmd, mp);
  write_values(&cmd, mp, values);
  cmd_send(cmd);

  return 0;
}

int8_t omlc_inject_metadata(OmlMP *mp, const char *key, const OmlValueU *value, OmlValueT type, const char *fname) {
  FrameT cmd;

  cmd_init(cmd, INJECT_METADATA);
  write_mp(&cmd, mp);
  write_string(&cmd, key);
  write_value(&cmd, type, value);
  write_string(&cmd, fname);
  cmd_send(cmd);
  
  return 0;
}

/* TODO: fabien -> get uid */
oml_guid_t omlc_guid_generate() {
  return 42;
}

void omlc_process(OmlMP* mp, OmlValueU* values) {
  FrameT cmd;

  cmd_init(cmd, PROCESS);
  write_mp(&cmd, mp);
  write_values(&cmd, mp, values);
}

int8_t omlc_close(void) {
  send_type(CLOSE);
  _OmlMP_clear_all();
  
  return 0;
}
