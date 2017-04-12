#ifndef USF_BPG_PKG_H
#define USF_BPG_PKG_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_bpg_pkg;

bpg_pkg_t bpg_pkg_create(bpg_pkg_manage_t mgr);
dp_req_t bpg_pkg_create_with_body(bpg_pkg_manage_t mgr, size_t capacity);
dp_req_t bpg_pkg_create_with_body_by_data_capacity(bpg_pkg_manage_t mgr, size_t capacity);
void bpg_pkg_free(bpg_pkg_t pkg);

bpg_pkg_manage_t bpg_pkg_mgr(bpg_pkg_t pkg);

dr_cvt_t bpg_pkg_data_cvt(bpg_pkg_t pkg);
dr_cvt_t bpg_pkg_base_cvt(bpg_pkg_t pkg);
LPDRMETA bpg_pkg_base_meta(bpg_pkg_t pkg);

dp_req_t bpg_pkg_to_dp_req(bpg_pkg_t pkg);
bpg_pkg_t bpg_pkg_find(dp_req_t req);

#ifdef __cplusplus
}
#endif

#endif
