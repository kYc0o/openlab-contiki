#include <stdio.h>
#include <stdlib.h>

#include "omlcore.h"

#define STR_MAX_NUM 8
#define STR_MAX_LEN 16

static char str_pool[STR_MAX_NUM][STR_MAX_LEN];
static uint8_t str_index = 0;

void _OmlString_copy(OmlString *oml_str, char const *str) {
  if (str) {
    oml_str->size = STR_MAX_LEN;
    oml_str->ptr = str_pool[str_index];
    oml_str->length = snprintf(oml_str->ptr, STR_MAX_LEN, str);
    str_index = (str_index + 1) % STR_MAX_NUM;
  }
}

void _OmlString_reset(OmlString *oml_str) {
  oml_str->size = 0;
  oml_str->length = 0;
  oml_str->ptr = NULL;
}