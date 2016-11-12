#ifndef CPE_POM_GRP_META_BUILD_H
#define CPE_POM_GRP_META_BUILD_H
#include "cpe/cfg/cfg_types.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_grp_meta_t
pom_grp_meta_build_from_cfg(
    mem_allocrator_t alloc,
    uint32_t omm_page_size,
    cfg_t cfg,
    LPDRMETALIB metalib,
    error_monitor_t em);

pom_grp_meta_t
pom_grp_meta_build_from_meta(
    mem_allocrator_t alloc,
    uint32_t omm_page_size,
    LPDRMETA dr_meta,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
