#ifndef GD_DR_STORE_MANAGE_H
#define GD_DR_STORE_MANAGE_H
#include "dr_store_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_store_manage_t
dr_store_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dr_store_manage_free(dr_store_manage_t mgr);

dr_store_manage_t
dr_store_manage_find(gd_app_context_t app, cpe_hash_string_t name);

dr_store_manage_t
dr_store_manage_find_nc(gd_app_context_t app, const char * name);

dr_store_manage_t
dr_store_manage_default(gd_app_context_t app);

gd_app_context_t dr_store_manage_app(dr_store_manage_t mgr);
const char * dr_store_manage_name(dr_store_manage_t mgr);
cpe_hash_string_t dr_store_manage_name_hs(dr_store_manage_t mgr);

LPDRMETA dr_store_manage_find_meta(dr_store_manage_t, const char * lib_and_name);
    
int dr_store_manage_load_from_bin(dr_store_manage_t mgr, const char * libname, const char * path);
    
#ifdef __cplusplus
}
#endif

#endif
