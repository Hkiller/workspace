#ifndef USF_BPG_USE_SP_H
#define USF_BPG_USE_SP_H
#include "cpe/cfg/cfg_types.h"
#include "bpg_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_use_sp_t
bpg_use_sp_create(
    gd_app_context_t app,
    const char * name,
    bpg_pkg_manage_t pkg_manage,
    mem_allocrator_t alloc, error_monitor_t em);

void bpg_use_sp_free(bpg_use_sp_t sp);

gd_app_context_t bpg_use_sp_app(bpg_use_sp_t sp);

bpg_use_sp_t bpg_use_sp_find(gd_app_context_t app, cpe_hash_string_t name);
bpg_use_sp_t bpg_use_sp_find_nc(gd_app_context_t app, const char * name);

const char * bpg_use_sp_name(bpg_use_sp_t sp);

uint64_t bpg_use_sp_client_id(bpg_use_sp_t sp);
void bpg_use_sp_set_client_id(bpg_use_sp_t sp, uint64_t client_id);

int bpg_use_sp_send(bpg_use_sp_t sp, dp_req_t pkg);

dp_req_t bpg_use_sp_pkg_buf(bpg_use_sp_t sp, size_t capacity);
void * bpg_use_sp_data_buf(bpg_use_sp_t sp, size_t capacity);

bpg_pkg_manage_t bpg_use_sp_pkg_manage(bpg_use_sp_t sp);
LPDRMETALIB bpg_use_sp_metalib(bpg_use_sp_t sp);
LPDRMETA bpg_use_sp_meta(bpg_use_sp_t sp, const char * name);

#ifdef __cplusplus
}
#endif

#endif
