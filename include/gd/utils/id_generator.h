#ifndef GD_UTILS_IDGENERATE_H
#define GD_UTILS_IDGENERATE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_id_generator_t
gd_id_generator_find(gd_app_context_t app, cpe_hash_string_t name);

gd_id_generator_t
gd_id_generator_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t gd_id_generator_app(gd_id_generator_t generator);
const char * gd_id_generator_name(gd_id_generator_t generator);
cpe_hash_string_t gd_id_generator_name_hs(gd_id_generator_t generator);

int gd_id_generator_generate(gd_id_t * r, gd_id_generator_t generator, const char * id_name);

#ifdef __cplusplus
}
#endif

#endif

