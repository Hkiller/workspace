#ifndef GD_UTILS_IDGENERATE_FILE_H
#define GD_UTILS_IDGENERATE_FILE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_id_file_generator_t
gd_id_file_generator_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void gd_id_file_generator_free(gd_id_file_generator_t mgr);

gd_id_file_generator_t
gd_id_file_generator_find(gd_app_context_t app, cpe_hash_string_t name);

gd_id_file_generator_t
gd_id_file_generator_find_nc(gd_app_context_t app, const char * name);

int gd_id_file_generator_generate(gd_id_t * r, gd_id_file_generator_t generator);

const char * gd_id_file_generator_save_dir(gd_id_file_generator_t generator);
int gd_id_file_generator_set_save_dir(gd_id_file_generator_t generator, const char * file);

uint32_t gd_id_file_generator_once_alloc_size(gd_id_file_generator_t generator);
void gd_id_file_generator_set_once_alloc_size(gd_id_file_generator_t generator, uint32_t once_alloc_size);

gd_app_context_t gd_id_file_generator_app(gd_id_file_generator_t generator);
const char * gd_id_file_generator_name(gd_id_file_generator_t generator);
cpe_hash_string_t gd_id_file_generator_name_hs(gd_id_file_generator_t generator);

#ifdef __cplusplus
}
#endif

#endif

