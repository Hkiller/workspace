#ifndef USF_BPG_RSP_PKG_BUILDER_H
#define USF_BPG_RSP_PKG_BUILDER_H
#include "bpg_rsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_rsp_pkg_builder_t
bpg_rsp_pkg_builder_create(bpg_rsp_manage_t mgr);

void bpg_rsp_pkg_builder_free(bpg_rsp_pkg_builder_t pkg_builder);

void bpg_rsp_pkg_builder_set_build(
    bpg_rsp_pkg_builder_t pkg_builder,
    bpg_pkg_build_fun_t build_fun,
    void * build_ctx);

#ifdef __cplusplus
}
#endif

#endif
