#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_rsp/bpg_rsp_pkg_builder.h"
#include "bpg_rsp_internal_ops.h"

static void bpg_rsp_manage_clear(nm_node_t node);

static cpe_hash_string_buf s_bpg_rsp_manage_default_name = CPE_HS_BUF_MAKE("bpg_rsp_manage");

struct nm_node_type s_nm_node_type_bpg_rsp_manage = {
    "usf_bpg_rsp_manage",
    bpg_rsp_manage_clear
};

bpg_rsp_manage_t
bpg_rsp_manage_create(
    gd_app_context_t app,
    const char * name,
    bpg_rsp_manage_dp_scope_t scope,
    logic_manage_t logic_mgr,
    logic_executor_mgr_t executor_mgr,
    bpg_pkg_manage_t pkg_manage,
    error_monitor_t em)
{
    bpg_rsp_manage_t mgr;
    nm_node_t mgr_node;

    assert(app);
    assert(logic_mgr);
    assert(executor_mgr);

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_bpg_rsp_manage_default_name);
    if (em == 0) em = gd_app_em(app);

    if (logic_mgr == 0) {
        CPE_ERROR(em, "%s: create: logic_mgr not exist!", name);
        return NULL;
    }

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_rsp_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_rsp_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = gd_app_alloc(app);
    mgr->m_logic_mgr = logic_mgr;
    mgr->m_pkg_manage = pkg_manage;
    mgr->m_executor_mgr = executor_mgr;
    mgr->m_em = em;
    mgr->m_flags = 0;

    mgr->m_ctx_capacity = 0;
    mgr->m_ctx_init = NULL;
    mgr->m_ctx_fini = NULL;
    mgr->m_pkg_init = NULL;
    mgr->m_ctx_ctx = NULL;
    mgr->m_dispatch_recv_at = NULL;

    mgr->m_rsp_max_size = 4 * 1024;
    mgr->m_rsp_buf = NULL;

    mgr->m_debug = 0;

    mgr->m_commit_dsp = NULL;
    mgr->m_forward_dsp = NULL;

    TAILQ_INIT(&mgr->m_pkg_builders);

    if (cpe_hash_table_init(
            &mgr->m_rsps,
            mgr->m_alloc,
            (cpe_hash_fun_t) bpg_rsp_hash,
            (cpe_hash_eq_t) bpg_rsp_cmp,
            CPE_HASH_OBJ2ENTRY(bpg_rsp, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_default_queue_info = NULL;
    if (cpe_hash_table_init(
            &mgr->m_queue_infos,
            mgr->m_alloc,
            (cpe_hash_fun_t) bpg_rsp_queue_info_hash,
            (cpe_hash_eq_t) bpg_rsp_queue_info_cmp,
            CPE_HASH_OBJ2ENTRY(bpg_rsp_queue_info, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_rsps);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (scope == bpg_rsp_manage_dp_scope_global) {
        mgr->m_dp = gd_app_dp_mgr(app);
    }
    else {
        mgr->m_dp = dp_mgr_create(gd_app_alloc(app));
        if (mgr->m_dp == NULL) {
            cpe_hash_table_fini(&mgr->m_queue_infos);
            cpe_hash_table_fini(&mgr->m_rsps);
            nm_node_free(mgr_node);
            return NULL;
        }
    }
    mgr->m_dp_req_buf = NULL;

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_rsp_manage);

    return mgr;
}

static void bpg_rsp_manage_clear(nm_node_t node) {
    char dsp_rsp_name[128];
    dp_rsp_t dsp_rsp;
    bpg_rsp_manage_t mgr;
    mgr = (bpg_rsp_manage_t)nm_node_data(node);

    if (mgr->m_dp_req_buf) {
        dp_req_free(mgr->m_dp_req_buf);
        mgr->m_dp_req_buf = NULL;
    }

    snprintf(dsp_rsp_name, sizeof(dsp_rsp_name), "%s.dp-rsp", bpg_rsp_manage_name(mgr));
    if ((dsp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), dsp_rsp_name))) {
        dp_rsp_free(dsp_rsp);
    }

    bpg_rsp_free_all(mgr);

    while(!TAILQ_EMPTY(&mgr->m_pkg_builders)) {
        bpg_rsp_pkg_builder_free(TAILQ_FIRST(&mgr->m_pkg_builders));
    }

    if (mgr->m_rsp_buf) {
        dp_req_free(mgr->m_rsp_buf);
        mgr->m_rsp_buf = NULL;
    }

    if (mgr->m_commit_dsp) {
        bpg_pkg_dsp_free(mgr->m_commit_dsp);
        mgr->m_commit_dsp = NULL;
    }

    if (mgr->m_forward_dsp) {
        bpg_pkg_dsp_free(mgr->m_forward_dsp);
        mgr->m_forward_dsp = NULL;
    }

    mgr->m_default_queue_info = NULL;
    bpg_rsp_queue_info_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_rsps);
    cpe_hash_table_fini(&mgr->m_queue_infos);

    if (mgr->m_dispatch_recv_at) {
        dp_rsp_t dp_rsp;

        dp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), mgr->m_dispatch_recv_at);
        if (dp_rsp) {
            dp_rsp_free(dp_rsp);
        }

        mem_free(mgr->m_alloc, (void*)mgr->m_dispatch_recv_at);
        mgr->m_dispatch_recv_at = NULL;
    }

    if (mgr->m_dp != gd_app_dp_mgr(mgr->m_app)) {
        dp_mgr_free(mgr->m_dp);
    }
    mgr->m_dp = NULL;
}

void bpg_rsp_manage_free(bpg_rsp_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_rsp_manage) return;
    nm_node_free(mgr_node);
}

bpg_rsp_manage_t
bpg_rsp_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) name = (cpe_hash_string_t)&s_bpg_rsp_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_rsp_manage) return NULL;
    return (bpg_rsp_manage_t)nm_node_data(node);
}

bpg_rsp_manage_t
bpg_rsp_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return bpg_rsp_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_rsp_manage) return NULL;
    return (bpg_rsp_manage_t)nm_node_data(node);
}

bpg_rsp_manage_t
bpg_rsp_manage_default(gd_app_context_t app) {
    return bpg_rsp_manage_find(app, (cpe_hash_string_t)&s_bpg_rsp_manage_default_name);
}

gd_app_context_t bpg_rsp_manage_app(bpg_rsp_manage_t mgr) {
    return mgr->m_app;
}

const char * bpg_rsp_manage_name(bpg_rsp_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
bpg_rsp_manage_name_hs(bpg_rsp_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

logic_manage_t bpg_rsp_manage_logic(bpg_rsp_manage_t mgr) {
    return mgr->m_logic_mgr;
}

bpg_pkg_dsp_t bpg_rsp_manage_commit_dsp(bpg_rsp_manage_t mgr) {
    return mgr->m_commit_dsp;
}

void bpg_rsp_manage_set_commit_dsp(bpg_rsp_manage_t mgr, bpg_pkg_dsp_t dsp) {
    if (mgr->m_commit_dsp) bpg_pkg_dsp_free(mgr->m_commit_dsp);
    mgr->m_commit_dsp = dsp;
}

bpg_pkg_dsp_t bpg_rsp_manage_forward_dsp(bpg_rsp_manage_t mgr) {
    return mgr->m_forward_dsp;
}

void bpg_rsp_manage_set_forward_dsp(bpg_rsp_manage_t mgr, bpg_pkg_dsp_t dsp) {
    if (mgr->m_forward_dsp) bpg_pkg_dsp_free(mgr->m_forward_dsp);
    mgr->m_forward_dsp = dsp;
}

static int bpg_rsp_manage_dispatch(dp_req_t dp_req, void * ctx, error_monitor_t em) {
    bpg_rsp_manage_t mgr = (bpg_rsp_manage_t)ctx;

    if (mgr->m_dp != dp_req_mgr(dp_req)) {
        if (mgr->m_dp_req_buf && (dp_req_capacity(mgr->m_dp_req_buf) < dp_req_size(dp_req))) {
            dp_req_free(mgr->m_dp_req_buf);
            mgr->m_dp_req_buf = NULL;
        }

        if (mgr->m_dp_req_buf == NULL) {
            mgr->m_dp_req_buf = bpg_pkg_create_with_body(mgr->m_pkg_manage, dp_req_size(dp_req));
            if (mgr->m_dp_req_buf == NULL) {
                CPE_ERROR(mgr->m_em, "%s: create dp_rsp buf fail", bpg_rsp_manage_name(mgr));
                return -1;
            }
        }

        memcpy(dp_req_data(mgr->m_dp_req_buf), dp_req_data(dp_req), dp_req_size(dp_req));
        dp_req_set_type(mgr->m_dp_req_buf, dp_req_type(dp_req));
        dp_req_set_size(mgr->m_dp_req_buf, dp_req_size(dp_req));
        dp_req_set_meta(mgr->m_dp_req_buf, dp_req_meta(dp_req));

        return dp_dispatch_by_numeric(bpg_pkg_cmd(dp_req), dp_req_mgr(mgr->m_dp_req_buf), mgr->m_dp_req_buf, em);
    }
    else {
        return dp_dispatch_by_numeric(bpg_pkg_cmd(dp_req), dp_req_mgr(mgr->m_dp_req_buf), dp_req, em);
    }
}

int bpg_rsp_manage_set_dispatch_at(bpg_rsp_manage_t mgr, const char * recv_at) {
    char dsp_rsp_name[128];
    dp_rsp_t dsp_rsp;

    snprintf(dsp_rsp_name, sizeof(dsp_rsp_name), "%s.dp-rsp", bpg_rsp_manage_name(mgr));
    if ((dsp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), dsp_rsp_name))) {
        dp_rsp_free(dsp_rsp);
        dsp_rsp = NULL;
    }

    if (mgr->m_dispatch_recv_at) {
        mem_free(mgr->m_alloc, (void*)mgr->m_dispatch_recv_at);
        mgr->m_dispatch_recv_at = NULL;
    }

    mgr->m_dispatch_recv_at = cpe_str_mem_dup(mgr->m_alloc, recv_at);
    if (mgr->m_dispatch_recv_at == NULL) return -1;

    dsp_rsp = dp_rsp_create(gd_app_dp_mgr(mgr->m_app), dsp_rsp_name);
    if (dsp_rsp == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create dp_rsp %s: create fail name duplicate!", bpg_rsp_manage_name(mgr), dsp_rsp_name);
        mem_free(mgr->m_alloc, (void*)mgr->m_dispatch_recv_at);
        mgr->m_dispatch_recv_at = NULL;
        return -1;
    }

    dp_rsp_set_processor(dsp_rsp, bpg_rsp_manage_dispatch, mgr);

    if (dp_rsp_bind_string(dsp_rsp, mgr->m_dispatch_recv_at, NULL) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: create dp_rsp %s: bind to %s fail!",
            bpg_rsp_manage_name(mgr), dsp_rsp_name, mgr->m_dispatch_recv_at);
        dp_rsp_free(dsp_rsp);
        mem_free(mgr->m_alloc, (void*)mgr->m_dispatch_recv_at);
        mgr->m_dispatch_recv_at = NULL;
        return -1;
    }

    return 0;
}

dp_req_t
bpg_rsp_manage_rsp_buf(bpg_rsp_manage_t mgr) {
    if (mgr->m_rsp_buf) {
        if (dp_req_capacity(mgr->m_rsp_buf) < mgr->m_rsp_max_size) {
            dp_req_free(mgr->m_rsp_buf);
            mgr->m_rsp_buf = NULL;
        }
    }

    if (mgr->m_rsp_buf == NULL) {
        mgr->m_rsp_buf = bpg_pkg_create_with_body(mgr->m_pkg_manage, mgr->m_rsp_max_size);
    }

    return mgr->m_rsp_buf;
}

uint32_t bpg_rsp_manage_flags(bpg_rsp_manage_t mgr) {
    return mgr->m_flags;
}

void bpg_rsp_manage_flags_set(bpg_rsp_manage_t mgr, uint32_t flag) {
    mgr->m_flags = flag;
}

void bpg_rsp_manage_flag_enable(bpg_rsp_manage_t mgr, bpg_rsp_manage_flag_t flag) {
    mgr->m_flags |= flag;
}

void bpg_rsp_manage_flag_disable(bpg_rsp_manage_t mgr, bpg_rsp_manage_flag_t flag) {
    mgr->m_flags &= ~((uint32_t)flag);
}

int bpg_rsp_manage_flag_is_enable(bpg_rsp_manage_t mgr, bpg_rsp_manage_flag_t flag) {
    return mgr->m_flags & flag;
}

void bpg_rsp_manage_set_context_op(
    bpg_rsp_manage_t mgr,
    size_t ctx_capacity,
    bpg_logic_ctx_init_fun_t ctx_init,
    bpg_logic_ctx_fini_fun_t ctx_fini,
    bpg_logic_pkg_init_fun_t pkg_init,
    void * ctx_ctx)
{
    mgr->m_ctx_capacity = ctx_capacity;
    mgr->m_ctx_init = ctx_init;
    mgr->m_ctx_fini = ctx_fini;
    mgr->m_pkg_init = pkg_init;
    mgr->m_ctx_ctx = ctx_ctx;
}
