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
#include "cpe/dr/dr_data.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "set_logic_rsp_ops.h"

static void set_logic_rsp_manage_clear(nm_node_t node);

static cpe_hash_string_buf s_set_logic_rsp_manage_default_name = CPE_HS_BUF_MAKE("set_logic_rsp_manage");

struct nm_node_type s_nm_node_type_set_logic_rsp_manage = {
    "usf_set_logic_rsp_manage",
    set_logic_rsp_manage_clear
};

set_logic_rsp_manage_t
set_logic_rsp_manage_create(
    gd_app_context_t app,
    const char * name,
    set_logic_rsp_manage_dp_scope_t scope,
    logic_manage_t logic_mgr,
    logic_executor_mgr_t executor_mgr,
    set_svr_stub_t stub,
    error_monitor_t em)
{
    set_logic_rsp_manage_t mgr;
    nm_node_t mgr_node;

    assert(app);
    assert(logic_mgr);
    assert(executor_mgr);

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_set_logic_rsp_manage_default_name);
    if (em == 0) em = gd_app_em(app);

    if (logic_mgr == 0) {
        CPE_ERROR(em, "%s: create: logic_mgr not exist!", name);
        return NULL;
    }

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct set_logic_rsp_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (set_logic_rsp_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = gd_app_alloc(app);
    mgr->m_stub = stub;
    mgr->m_logic_mgr = logic_mgr;
    mgr->m_executor_mgr = executor_mgr;
    mgr->m_em = em;
    mgr->m_flags = 0;
    mgr->m_queue_attr = NULL;

    mgr->m_svr_type = set_svr_stub_svr_type(stub);
    assert(mgr->m_svr_type);

    mgr->m_pkg_meta = set_svr_svr_info_pkg_meta(mgr->m_svr_type);
    if (mgr->m_pkg_meta == NULL) {
        CPE_ERROR(em, "%s: create: no pkg_meta!", name);
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_pkg_cmd_entry = set_svr_svr_info_pkg_cmd_entry(set_svr_stub_svr_type(stub));
    if (mgr->m_pkg_cmd_entry == NULL) {
        CPE_ERROR(em, "%s: create: no pkg_cmd_entry!", name);
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_pkg_data_entry = set_svr_svr_info_pkg_data_entry(set_svr_stub_svr_type(stub));
    if (mgr->m_pkg_data_entry == NULL) {
        CPE_ERROR(em, "%s: create: no pkg_data_entry!", name);
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_ctx_capacity = 0;
    mgr->m_ctx_init = NULL;
    mgr->m_ctx_fini = NULL;
    mgr->m_ctx_ctx = NULL;

    mgr->m_rsp_buf = NULL;

    mgr->m_debug = 0;

    mgr->m_commit_to = NULL;
    mgr->m_recv_at = NULL;

    if (cpe_hash_table_init(
            &mgr->m_rsps,
            mgr->m_alloc,
            (cpe_hash_fun_t) set_logic_rsp_hash,
            (cpe_hash_eq_t) set_logic_rsp_cmp,
            CPE_HASH_OBJ2ENTRY(set_logic_rsp, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_default_queue_info = NULL;
    if (cpe_hash_table_init(
            &mgr->m_queue_infos,
            mgr->m_alloc,
            (cpe_hash_fun_t) set_logic_rsp_queue_info_hash,
            (cpe_hash_eq_t) set_logic_rsp_queue_info_cmp,
            CPE_HASH_OBJ2ENTRY(set_logic_rsp_queue_info, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_rsps);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (scope == set_logic_rsp_manage_dp_scope_global) {
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

    nm_node_set_type(mgr_node, &s_nm_node_type_set_logic_rsp_manage);

    return mgr;
}

static void set_logic_rsp_manage_clear(nm_node_t node) {
    char dsp_rsp_name[128];
    dp_rsp_t dsp_rsp;
    set_logic_rsp_manage_t mgr;
    mgr = (set_logic_rsp_manage_t)nm_node_data(node);

    snprintf(dsp_rsp_name, sizeof(dsp_rsp_name), "%s.dp-rsp", set_logic_rsp_manage_name(mgr));
    if ((dsp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), dsp_rsp_name))) {
        dp_rsp_free(dsp_rsp);
    }

    set_logic_rsp_free_all(mgr);

    if (mgr->m_rsp_buf) {
        dp_req_free(mgr->m_rsp_buf);
        mgr->m_rsp_buf = NULL;
    }

    mgr->m_default_queue_info = NULL;
    set_logic_rsp_queue_info_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_rsps);
    cpe_hash_table_fini(&mgr->m_queue_infos);

    mgr->m_dp = NULL;

    if (mgr->m_queue_attr) {
        mem_free(mgr->m_alloc, mgr->m_queue_attr);
        mgr->m_queue_attr = NULL;
    }

    if (mgr->m_recv_at) {
        dp_rsp_t dp_rsp;

        dp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), mgr->m_recv_at);
        if (dp_rsp) {
            dp_rsp_free(dp_rsp);
        }

        mem_free(mgr->m_alloc, (void*)mgr->m_recv_at);
        mgr->m_recv_at = NULL;
    }

    if (mgr->m_commit_to) {
        mem_free(mgr->m_alloc, mgr->m_commit_to);
        mgr->m_commit_to = NULL;
    }
}

void set_logic_rsp_manage_free(set_logic_rsp_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_set_logic_rsp_manage) return;
    nm_node_free(mgr_node);
}

set_logic_rsp_manage_t
set_logic_rsp_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) name = (cpe_hash_string_t)&s_set_logic_rsp_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_logic_rsp_manage) return NULL;
    return (set_logic_rsp_manage_t)nm_node_data(node);
}

set_logic_rsp_manage_t
set_logic_rsp_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return set_logic_rsp_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_logic_rsp_manage) return NULL;
    return (set_logic_rsp_manage_t)nm_node_data(node);
}

set_logic_rsp_manage_t
set_logic_rsp_manage_default(gd_app_context_t app) {
    return set_logic_rsp_manage_find(app, (cpe_hash_string_t)&s_set_logic_rsp_manage_default_name);
}

gd_app_context_t set_logic_rsp_manage_app(set_logic_rsp_manage_t mgr) {
    return mgr->m_app;
}

const char * set_logic_rsp_manage_name(set_logic_rsp_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
set_logic_rsp_manage_name_hs(set_logic_rsp_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

logic_manage_t set_logic_rsp_manage_logic(set_logic_rsp_manage_t mgr) {
    return mgr->m_logic_mgr;
}

dp_req_t
set_logic_rsp_manage_rsp_buf(set_logic_rsp_manage_t mgr, size_t data_capacity) {
    if (mgr->m_rsp_buf) {
        if (dp_req_capacity(mgr->m_rsp_buf) < data_capacity) {
            dp_req_free(mgr->m_rsp_buf);
            mgr->m_rsp_buf = NULL;
        }
    }

    if (mgr->m_rsp_buf == NULL) {
        mgr->m_rsp_buf = dp_req_create(mgr->m_dp, data_capacity);
    }

    return mgr->m_rsp_buf;
}

uint32_t set_logic_rsp_manage_flags(set_logic_rsp_manage_t mgr) {
    return mgr->m_flags;
}

void set_logic_rsp_manage_flags_set(set_logic_rsp_manage_t mgr, uint32_t flag) {
    mgr->m_flags = flag;
}

void set_logic_rsp_manage_flag_enable(set_logic_rsp_manage_t mgr, set_logic_rsp_manage_flag_t flag) {
    mgr->m_flags |= flag;
}

void set_logic_rsp_manage_flag_disable(set_logic_rsp_manage_t mgr, set_logic_rsp_manage_flag_t flag) {
    mgr->m_flags &= ~((uint32_t)flag);
}

int set_logic_rsp_manage_flag_is_enable(set_logic_rsp_manage_t mgr, set_logic_rsp_manage_flag_t flag) {
    return mgr->m_flags & flag;
}

static int set_logic_rsp_manage_dispatch(dp_req_t dp_req, void * ctx, error_monitor_t em) {
    uint32_t cmd;
    set_logic_rsp_manage_t mgr = (set_logic_rsp_manage_t)ctx;

    if (dr_entry_try_read_uint32(
            &cmd, ((const char*)dp_req_data(dp_req)) + dr_entry_data_start_pos(mgr->m_pkg_cmd_entry, 0),
            mgr->m_pkg_cmd_entry, em) != 0)
    {
        CPE_ERROR(
            em, "%s: set_logic_rsp_manage_dispatch: read cmd from %s fail!",
            set_logic_rsp_manage_name(mgr), dr_entry_name(mgr->m_pkg_cmd_entry));
        return -1;
    }

    return dp_dispatch_by_numeric(cmd, mgr->m_dp, dp_req, em);
}

int set_logic_rsp_manage_set_recv_at(set_logic_rsp_manage_t mgr, const char * recv_at) {
    char dsp_rsp_name[128];
    dp_rsp_t dsp_rsp;

    snprintf(dsp_rsp_name, sizeof(dsp_rsp_name), "%s.dp-rsp", set_logic_rsp_manage_name(mgr));
    if ((dsp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), dsp_rsp_name))) {
        dp_rsp_free(dsp_rsp);
        dsp_rsp = NULL;
    }

    if (mgr->m_recv_at) {
        mem_free(mgr->m_alloc, (void*)mgr->m_recv_at);
        mgr->m_recv_at = NULL;
    }

    mgr->m_recv_at = cpe_str_mem_dup(mgr->m_alloc, recv_at);
    if (mgr->m_recv_at == NULL) return -1;

    dsp_rsp = dp_rsp_create(gd_app_dp_mgr(mgr->m_app), dsp_rsp_name);
    if (dsp_rsp == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create dp_rsp %s: create fail name duplicate!", set_logic_rsp_manage_name(mgr), dsp_rsp_name);
        mem_free(mgr->m_alloc, (void*)mgr->m_recv_at);
        mgr->m_recv_at = NULL;
        return -1;
    }

    dp_rsp_set_processor(dsp_rsp, set_logic_rsp_manage_dispatch, mgr);

    if (dp_rsp_bind_string(dsp_rsp, mgr->m_recv_at, NULL) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: create dp_rsp %s: bind to %s fail!",
            set_logic_rsp_manage_name(mgr), dsp_rsp_name, mgr->m_recv_at);
        dp_rsp_free(dsp_rsp);
        mem_free(mgr->m_alloc, (void*)mgr->m_recv_at);
        mgr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}

void set_logic_rsp_manage_set_context_op(
    set_logic_rsp_manage_t mgr,
    size_t ctx_capacity,
    set_logic_ctx_init_fun_t ctx_init,
    set_logic_ctx_fini_fun_t ctx_fini,
    void * ctx_ctx)
{
    mgr->m_ctx_capacity = ctx_capacity;
    mgr->m_ctx_init = ctx_init;
    mgr->m_ctx_fini = ctx_fini;
    mgr->m_ctx_ctx = ctx_ctx;
}
