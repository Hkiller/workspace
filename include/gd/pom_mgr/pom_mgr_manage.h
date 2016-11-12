#ifndef GD_POM_MGR_MANAGE_H
#define GD_POM_MGR_MANAGE_H
#include "pom_mgr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_manage_t
pom_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void pom_manage_free(pom_manage_t mgr);

pom_manage_t
pom_manage_find(gd_app_context_t app, cpe_hash_string_t name);

pom_manage_t
pom_manage_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t pom_manage_app(pom_manage_t mgr);
const char * pom_manage_name(pom_manage_t mgr);
cpe_hash_string_t pom_manage_name_hs(pom_manage_t mgr);

pom_grp_obj_mgr_t pom_manage_obj_mgr(pom_manage_t mgr);

int pom_manage_obj_mgr_init_mem(pom_manage_t mgr, LPDRMETALIB metalib, pom_grp_meta_t meta, size_t capacity);
int pom_manage_obj_mgr_load_shm(pom_manage_t mgr, LPDRMETALIB metalib, pom_grp_meta_t meta, size_t capacity);
void pom_manage_obj_mgr_clear(pom_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
