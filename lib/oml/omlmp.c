#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omlcore.h"
#include "omlmp.h"

#define MAX_MP 16
#define MAX_NAME_LEN 64

static OmlMP mps_pool[MAX_MP];
static char mps_names[MAX_MP][MAX_NAME_LEN];
static uint8_t mps_count = 0;

void _OmlMP_clear_all() {
  mps_count = 0;
}

OmlMP* _OmlMP_get() {
  if (mps_count < MAX_MP) {
    return mps_pool + mps_count++;
  } else {
    return NULL;
  }
}

char* _OmlMP_mpname_cpy(const char* mpname) {
  char* cpy;

  cpy = mps_names[mps_count - 1];
  memset(cpy, 0, MAX_NAME_LEN);
  snprintf(cpy, MAX_NAME_LEN, mpname);
  return cpy;
}

void _OmlMP_set_param(OmlMP *mp, OmlMPDef *mp_def) {
  mp->param_count = 0;
  mp->param_defs = mp_def;
  while (mp_def && mp_def->name) {
    mp->param_count++;
    mp_def++;
  }
}

OmlMP* _OmlMP_create(const char *mp_name, OmlMPDef *mp_def) {
  OmlMP *mp;
  
  mp = _OmlMP_get();
  if (mp) {
    memset(mp, 0, sizeof(OmlMP));
    mp->name = _OmlMP_mpname_cpy(mp_name);
    _OmlMP_set_param(mp, mp_def);
  }

  return mp;
}
