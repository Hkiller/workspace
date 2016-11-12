#ifndef USF_BPG_USE_PKG_CHANEL_H
#define USF_BPG_USE_PKG_CHANEL_H
#include "cpe/utils/memory.h"
#include "usf/bpg_pkg/bpg_pkg_types.h" 
#include "bpg_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_use_pkg_chanel_t
bpg_use_pkg_chanel_create(
    gd_app_context_t app,
    const char * name,
    bpg_pkg_manage_t pkg_manage,
    mem_allocrator_t alloc,
    error_monitor_t em);

void bpg_use_pkg_chanel_free(bpg_use_pkg_chanel_t pkg_chanel);

gd_app_context_t bpg_use_pkg_chanel_app(bpg_use_pkg_chanel_t pkg_chanel);
const char * bpg_use_pkg_chanel_name(bpg_use_pkg_chanel_t pkg_chanel);

bpg_pkg_manage_t bpg_use_pkg_chanel_pkg_manage(bpg_use_pkg_chanel_t pkg_chanel);

#ifdef __cplusplus
}
#endif

#endif
