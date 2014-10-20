#ifndef OML_OMLCORE_H_
#define OML_OMLCORE_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t oml_guid_t;

typedef enum OmlValueT {
  /* Meta value types */
  OML_DB_PRIMARY_KEY = -3,
  OML_INPUT_VALUE = -2,
  OML_UNKNOWN_VALUE = -1,
  /* Concrete value types */
  OML_DOUBLE_VALUE = 0,
  OML_LONG_VALUE,
  OML_PADDING1_VALUE,
  OML_STRING_VALUE,
  OML_INT32_VALUE,
  OML_UINT32_VALUE,
  OML_INT64_VALUE,
  OML_UINT64_VALUE,
  OML_BLOB_VALUE,
  OML_GUID_VALUE,
  OML_BOOL_VALUE,
  OML_LAST_VALUE /* For easy range checks */
} OmlValueT;

typedef struct OmlString {
  char *ptr;
  uint8_t length;
  uint8_t size;
  int32_t is_const;
} OmlString;

typedef struct OmlBlob {
  void *ptr;
  uint8_t length;
  uint8_t size;
} OmlBlob;

typedef union OmlValueU {
  int64_t   longValue;
  double    doubleValue;
  OmlString stringValue;
  int32_t   int32Value;
  uint32_t  uint32Value;
  int64_t   int64Value;
  uint64_t  uint64Value;
  OmlBlob   blobValue;
  oml_guid_t    guidValue;
  uint8_t   boolValue;
} OmlValueU;

#define omlc_zero_array(var, n) \
memset((var), 0, n*sizeof(OmlValueU))

#define _omlc_set_intrinsic_value(var, type, val) \
((var).type ## Value = (val))

#define omlc_set_int32(var, val) \
_omlc_set_intrinsic_value(var, int32, (int32_t)(val))

#define omlc_set_uint32(var, val) \
_omlc_set_intrinsic_value(var, uint32, (uint32_t)(val))

#define omlc_set_int64(var, val) \
_omlc_set_intrinsic_value(var, int64, (int64_t)(val))

#define omlc_set_uint64(var, val) \
_omlc_set_intrinsic_value(var, int64, (uint64_t)(val))

#define omlc_set_double(var, val) \
_omlc_set_intrinsic_value(var, double, (double)(val))

#define omlc_set_long(var, val) \
_omlc_set_intrinsic_value(var, long, (int64_t)(val))

#define omlc_set_guid(var, val)                            \
_omlc_set_intrinsic_value(var, guid, (oml_guid_t)(val))

#define omlc_set_bool(var, val)                            \
_omlc_set_intrinsic_value(var, bool, (val != OMLC_BOOL_FALSE))

#define omlc_reset_string(var)                                  \
  do {                                                          \
    _OmlString_reset(&((var).stringValue));                     \
  } while(0)

#define omlc_set_string(var, str)                               \
  do {                                                          \
    omlc_reset_string(var);                                     \
    _OmlString_copy(&((var).stringValue), str);                 \
  } while(0)

typedef struct OmlMPDef {
  const char*   name;
  OmlValueT     param_types;
} OmlMPDef;

typedef struct OmlMP {
  const char*   name;
  OmlMPDef*     param_defs;
  uint8_t       param_count;
  struct OmlMP* next;
} OmlMP;

/**
 * Internal utils functions
 * Do NOT use these directly
 */

void _OmlString_copy(OmlString *oml_str, char const *str);

void _OmlString_reset(OmlString *oml_str);

#ifdef __cplusplus
}
#endif

#endif /*OML_OMLCORE_H_*/