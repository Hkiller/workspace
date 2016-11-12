#ifndef USF_BPG_PKG_MANAGE_H
#define USF_BPG_PKG_MANAGE_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_pkg_manage_t
bpg_pkg_manage_create(
    gd_app_context_t app,
    const char * name,
    error_monitor_t em);

void bpg_pkg_manage_free(bpg_pkg_manage_t mgr);

bpg_pkg_manage_t
bpg_pkg_manage_find(gd_app_context_t app, cpe_hash_string_t name);

bpg_pkg_manage_t
bpg_pkg_manage_find_nc(gd_app_context_t app, const char * name);

bpg_pkg_manage_t
bpg_pkg_manage_default(gd_app_context_t app);

gd_app_context_t bpg_pkg_manage_app(bpg_pkg_manage_t mgr);
const char * bpg_pkg_manage_name(bpg_pkg_manage_t mgr);
cpe_hash_string_t bpg_pkg_manage_name_hs(bpg_pkg_manage_t mgr);

int bpg_pkg_manage_set_data_cvt(bpg_pkg_manage_t pkg, const char * cvt_name);
const char * bpg_pkg_manage_data_cvt_name(bpg_pkg_manage_t pkg);
dr_cvt_t bpg_pkg_manage_data_cvt(bpg_pkg_manage_t pkg);

int bpg_pkg_manage_set_base_cvt(bpg_pkg_manage_t pkg, const char * cvt_name);
const char * bpg_pkg_manage_base_cvt_name(bpg_pkg_manage_t pkg);
dr_cvt_t bpg_pkg_manage_base_cvt(bpg_pkg_manage_t pkg);

LPDRMETALIB bpg_pkg_manage_basepkg_metalib(bpg_pkg_manage_t mgr);
LPDRMETA bpg_pkg_manage_basepkg_meta(bpg_pkg_manage_t mgr);
LPDRMETA bpg_pkg_manage_basepkg_head_meta(bpg_pkg_manage_t mgr);

int bpg_pkg_manage_set_op_buff_capacity(bpg_pkg_manage_t mgr, size_t buf_size);

int bpg_pkg_manage_set_data_metalib(bpg_pkg_manage_t mgr, const char * metalib_name);
int bpg_pkg_manage_add_cmd_by_meta(bpg_pkg_manage_t mgr, const char * name);
int bpg_pkg_manage_add_cmd(bpg_pkg_manage_t mgr, uint32_t cmd, const char * name);

const char * bpg_pkg_manage_data_metalib_name(bpg_pkg_manage_t mgr);
LPDRMETALIB bpg_pkg_manage_data_metalib(bpg_pkg_manage_t mgr);

LPDRMETA bpg_pkg_manage_find_meta_by_cmd(bpg_pkg_manage_t mgr, uint32_t cmd);
int bpg_pkg_find_cmd_from_meta_name(uint32_t * cmd, bpg_pkg_manage_t mgr, const char * meta_name);

bpg_pkg_debug_level_t bpg_pkg_manage_debug_level(bpg_pkg_manage_t mgr, uint32_t cmd);
void bpg_pkg_manage_set_debug_level(bpg_pkg_manage_t mgr, uint32_t cmd, bpg_pkg_debug_level_t level);

uint32_t bpg_pkg_zip_size_threshold(bpg_pkg_manage_t mgr);
void bpg_pkg_set_zip_size_threshold(bpg_pkg_manage_t mgr, uint32_t threaded);

#ifdef __cplusplus
}
#endif

#endif
