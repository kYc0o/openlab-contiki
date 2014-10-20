#ifndef OML_OMLC_H_
#define OML_OMLC_H_

#include <stdint.h>

#include "omlcore.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*o_log_fn)(uint8_t log_level, const char* format, ...);

/**
 * Initialise the measurement library.
 */
int8_t omlc_init(const char *appName);

/**
 * Register a measurement point.
 */
OmlMP *omlc_add_mp(const char *mp_name, OmlMPDef *mp_def);

/**
 * Get ready to start the measurement collection.
 */
int8_t omlc_start(void);

/**
 * Inject a measurement sample into a Measurement Point.
 */
int8_t omlc_inject(OmlMP *mp, OmlValueU *values);

/**
 * Inject metadata (key/value) for a specific MP.
 */
int8_t omlc_inject_metadata(OmlMP *mp, const char *key, const OmlValueU *value, OmlValueT type, const char *fname);

/**
 * Generate a new GUID
 */
oml_guid_t omlc_guid_generate();

/**
 * DEPRECATED \see omlc_inject
 */
void omlc_process(OmlMP* mp, OmlValueU* values);

/**
 * Terminate all open connections.
 */
int8_t omlc_close(void);

#ifdef __cplusplus
}
#endif

#endif /*OML_OMLC_H_*/
