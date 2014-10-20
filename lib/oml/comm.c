#include <string.h>

#include "comm.h"
#include "omlcore.h"

/* TODO: check buffer overflow */
void write_bytes(FrameT *cmd, const void *source, uint8_t len) {
  memcpy(cmd->payload + cmd->len - 1, source, len);
  cmd->len += len;
}

/* TODO: check buffer overflow */
void write_byte(FrameT *cmd, uint8_t n) {
  cmd->payload[cmd->len++ - 1] = n;
}

void write_string(FrameT *cmd, const char* str) {
  uint8_t len;
  
  len = strlen(str);
  write_byte(cmd, len);
  write_bytes(cmd, str, len);
}

void write_mp(FrameT *cmd, const OmlMP *mp) {
  write_string(cmd, mp->name);
}

void write_mpdef(FrameT *cmd, const OmlMPDef *mpdef, uint8_t param_count) {
  uint8_t i;
  
  write_byte(cmd, param_count);
  for (i = 0; i < param_count; ++i) {
    write_string(cmd, mpdef[i].name);
    write_byte(cmd, mpdef[i].param_types);
  }
}

void write_OmlString(FrameT *cmd, const OmlString *str) {
  write_byte(cmd, str->length);
  write_bytes(cmd, str->ptr, str->length);
}

void write_OmlBlob(FrameT *cmd, const OmlBlob *blob) {
  write_byte(cmd, blob->length);
  write_bytes(cmd, blob->ptr, blob->length);
}

void write_value(FrameT *cmd, OmlValueT type, const OmlValueU *value) {
  switch (type) {
    case OML_STRING_VALUE:
      write_OmlString(cmd, &value->stringValue);
      break;
    case OML_BLOB_VALUE:
      write_OmlBlob(cmd, &value->blobValue);
      break;
    case OML_DOUBLE_VALUE:
      write_bytes(cmd, &value->doubleValue, sizeof(double));
      break;
    case OML_LONG_VALUE:
      write_bytes(cmd, &value->longValue, sizeof(int64_t));
      break;
    case OML_INT32_VALUE:
      write_bytes(cmd, &value->int32Value, sizeof(int32_t));
      break;
    case OML_UINT32_VALUE:
      write_bytes(cmd, &value->uint32Value, sizeof(uint32_t));
      break;
    case OML_INT64_VALUE:
      write_bytes(cmd, &value->int64Value, sizeof(int64_t));
      break;
    case OML_UINT64_VALUE:
      write_bytes(cmd, &value->uint64Value, sizeof(uint64_t));
      break;
    case OML_GUID_VALUE:
      write_bytes(cmd, &value->guidValue, sizeof(oml_guid_t));
      break;
    case OML_BOOL_VALUE:
      write_bytes(cmd, &value->boolValue, sizeof(uint8_t));
      break;
    default:
      write_bytes(cmd, value, sizeof(OmlValueU));
      break;
  }
}

void write_values(FrameT *cmd, OmlMP *mp, const OmlValueU *values) {
  OmlMPDef *defs;

  for (defs = mp->param_defs; defs->name; ++defs) {
    write_value(cmd, defs->param_types, values++);
  } 
}
