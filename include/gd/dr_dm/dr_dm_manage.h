#ifndef GD_DR_DM_MANAGE_H
#define GD_DR_DM_MANAGE_H
#include "cpe/utils/hash_string.h"
#include "gd/dr_store/dr_store_types.h"
#include "dr_dm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_dm_manage_t
dr_dm_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dr_dm_manage_free(dr_dm_manage_t mgr);

dr_dm_manage_t
dr_dm_manage_find(gd_app_context_t app, cpe_hash_string_t name);

dr_dm_manage_t
dr_dm_manage_find_nc(gd_app_context_t app, const char * name);

dr_dm_manage_t
dr_dm_manage_default(gd_app_context_t app);

gd_app_context_t dr_dm_manage_app(dr_dm_manage_t mgr);
const char * dr_dm_manage_name(dr_dm_manage_t mgr);
cpe_hash_string_t dr_dm_manage_name_hs(dr_dm_manage_t mgr);

LPDRMETA dr_dm_manage_meta(dr_dm_manage_t mgr);
int dr_dm_manage_set_meta(dr_dm_manage_t mgr, LPDRMETA meta, dr_ref_t metalib);

int dr_dm_manage_set_id_attr(dr_dm_manage_t mgr, const char * id_attr_name);
LPDRMETAENTRY dr_dm_manage_id_attr(dr_dm_manage_t mgr);

void dr_dm_manage_set_id_generate(dr_dm_manage_t mgr, gd_id_generator_t id_generate);
gd_id_generator_t dr_dm_manage_id_generate(dr_dm_manage_t mgr);

int dr_dm_manage_create_index(dr_dm_manage_t mgr, const char * name, int is_uniqure);

#ifdef __cplusplus
}
#endif

#endif
