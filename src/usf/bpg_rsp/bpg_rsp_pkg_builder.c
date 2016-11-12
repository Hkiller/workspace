#include "usf/bpg_rsp/bpg_rsp_pkg_builder.h"
#include "bpg_rsp_internal_ops.h"

bpg_rsp_pkg_builder_t
bpg_rsp_pkg_builder_create(bpg_rsp_manage_t mgr) {
    bpg_rsp_pkg_builder_t pkg_builder;

    pkg_builder =
        (bpg_rsp_pkg_builder_t)mem_alloc(mgr->m_alloc, sizeof(struct bpg_rsp_pkg_builder));
    if (pkg_builder == NULL) return NULL;

    pkg_builder->m_mgr = mgr;
    pkg_builder->m_build_fun = NULL;
    pkg_builder->m_build_ctx = NULL;

    TAILQ_INSERT_TAIL(&pkg_builder->m_mgr->m_pkg_builders, pkg_builder, m_next);

    return pkg_builder;
}

void bpg_rsp_pkg_builder_free(bpg_rsp_pkg_builder_t pkg_builder) {
    TAILQ_REMOVE(&pkg_builder->m_mgr->m_pkg_builders, pkg_builder, m_next);
    mem_free(pkg_builder->m_mgr->m_alloc, pkg_builder);
}

void bpg_rsp_pkg_builder_set_build(
    bpg_rsp_pkg_builder_t pkg_builder,
    bpg_pkg_build_fun_t build_fun,
    void * build_ctx)
{
    pkg_builder->m_build_fun = build_fun;
    pkg_builder->m_build_ctx = build_ctx;
}

