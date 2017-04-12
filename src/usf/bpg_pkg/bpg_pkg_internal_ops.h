#ifndef USF_BPG_PKG_INTERNAL_OPS_H
#define USF_BPG_PKG_INTERNAL_OPS_H
#include "bpg_pkg_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*debug info*/
uint32_t bpg_pkg_debug_info_hash(const struct bpg_pkg_debug_info * o);
int bpg_pkg_debug_info_cmp(const struct bpg_pkg_debug_info * l, const struct bpg_pkg_debug_info * r);
void bpg_pkg_debug_info_free_all(bpg_pkg_manage_t mgr);

/*cmd info*/
struct bpg_pkg_cmd_info * bpg_pkg_cmd_info_create(bpg_pkg_manage_t mgr, uint32_t cmd, LPDRMETA meta);
void bpg_pkg_cmd_info_free_all(bpg_pkg_manage_t mgr);

uint32_t bpg_pkg_cmd_info_cmd_hash(const struct bpg_pkg_cmd_info * o);
int bpg_pkg_cmd_info_cmd_eq(const struct bpg_pkg_cmd_info * l, const struct bpg_pkg_cmd_info * r);

uint32_t bpg_pkg_cmd_info_name_hash(const struct bpg_pkg_cmd_info * o);
int bpg_pkg_cmd_info_name_eq(const struct bpg_pkg_cmd_info * l, const struct bpg_pkg_cmd_info * r);

void * bpg_pkg_op_buff(bpg_pkg_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
