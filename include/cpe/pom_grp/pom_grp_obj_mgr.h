#ifndef CPE_POM_GRP_MANAGE_H
#define CPE_POM_GRP_MANAGE_H
#include "cpe/utils/stream.h"
#include "cpe/dr/dr_types.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_grp_obj_mgr_t
pom_grp_obj_mgr_create(
    mem_allocrator_t alloc,
    void * data,
    size_t data_capacity,
    error_monitor_t em);

void pom_grp_obj_mgr_free(pom_grp_obj_mgr_t mgr);

void * pom_grp_obj_mgr_data(pom_grp_obj_mgr_t mgr);
size_t pom_grp_obj_mgr_data_capacity(pom_grp_obj_mgr_t mgr);

pom_mgr_t pom_grp_obj_mgr_pom(pom_grp_obj_mgr_t mgr);
pom_grp_meta_t pom_grp_obj_mgr_meta(pom_grp_obj_mgr_t mgr);

int pom_grp_obj_mgr_buf_init(
    LPDRMETALIB metalib,
    pom_grp_meta_t grp_meta,
    void * data, size_t data_capacity,
    error_monitor_t em);

void pom_grp_obj_mgr_info(pom_grp_obj_mgr_t mgr, write_stream_t stream, int ident);

void pom_grp_obj_mgr_set_auto_validate(pom_grp_obj_mgr_t mgr, int auto_validate);
int pom_grp_obj_mgr_auto_validate(pom_grp_obj_mgr_t mgr);

int pom_grp_obj_mgr_validate(pom_grp_obj_mgr_t mgr, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
