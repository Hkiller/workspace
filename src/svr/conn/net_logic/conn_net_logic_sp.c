#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "svr/conn/net_cli/conn_net_cli_pkg.h"
#include "svr/conn/net_logic/conn_net_logic_sp.h"
#include "conn_net_logic_internal_ops.h"

static void conn_net_logic_sp_clear(nm_node_t node);
extern char g_metalib_conn_net_logic_pkg_info[];

struct nm_node_type s_nm_node_type_conn_net_logic_sp = {
    "svr_conn_net_logic_sp",
    conn_net_logic_sp_clear
};

conn_net_logic_sp_t
conn_net_logic_sp_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct conn_net_logic_sp * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct conn_net_logic_sp));
    if (mgr_node == NULL) return NULL;

    mgr = (conn_net_logic_sp_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_outgoing_dispatch_to = NULL;
    mgr->m_incoming_recv_at = NULL;
    mgr->m_outgoing_pkg = NULL;
    mgr->m_outgoing_body = NULL;

    mgr->m_require_queue = logic_require_queue_create(app, alloc, em, name, logic_mgr, 0);
    if (mgr->m_require_queue == NULL) {
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_conn_net_logic_sp);

    return mgr;
}

static void conn_net_logic_sp_clear(nm_node_t node) {
    conn_net_logic_sp_t mgr;
    mgr = (conn_net_logic_sp_t)nm_node_data(node);

    if (mgr->m_outgoing_dispatch_to) {
        mem_free(mgr->m_alloc, mgr->m_outgoing_dispatch_to);
        mgr->m_outgoing_dispatch_to = NULL;
    }

    if (mgr->m_incoming_recv_at) {
        dp_rsp_free(mgr->m_incoming_recv_at);
        mgr->m_incoming_recv_at = NULL;
    }

    logic_require_queue_free(mgr->m_require_queue);
    mgr->m_require_queue = NULL;
}

gd_app_context_t conn_net_logic_sp_app(conn_net_logic_sp_t mgr) {
    return mgr->m_app;
}

void conn_net_logic_sp_free(conn_net_logic_sp_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_conn_net_logic_sp) return;
    nm_node_free(mgr_node);
}

conn_net_logic_sp_t
conn_net_logic_sp_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_net_logic_sp) return NULL;
    return (conn_net_logic_sp_t)nm_node_data(node);
}

conn_net_logic_sp_t
conn_net_logic_sp_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "conn_net_logic_sp";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_net_logic_sp) return NULL;
    return (conn_net_logic_sp_t)nm_node_data(node);
}

const char * conn_net_logic_sp_name(conn_net_logic_sp_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
conn_net_logic_sp_name_hs(conn_net_logic_sp_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int conn_net_logic_sp_set_outgoing_dispatch_to(conn_net_logic_sp_t sp, const char * outgoing_dispatch_to) {
    cpe_hash_string_t new_outgoing_dispatch_to = cpe_hs_create(sp->m_alloc, outgoing_dispatch_to);
    if (new_outgoing_dispatch_to == NULL) return -1;

    if (sp->m_outgoing_dispatch_to) mem_free(sp->m_alloc, sp->m_outgoing_dispatch_to);
    sp->m_outgoing_dispatch_to = new_outgoing_dispatch_to;

    return 0;
}

cpe_hash_string_t conn_net_logic_sp_outgoing_dispatch_to(conn_net_logic_sp_t sp) {
    return sp->m_outgoing_dispatch_to;
}

static int conn_net_logic_sp_incoming_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_net_logic_sp_t sp = ctx;
    conn_net_cli_pkg_t pkg = conn_net_cli_pkg_find(req);
    logic_require_t require;
    logic_data_t data;

    require = logic_require_queue_remove_get(sp->m_require_queue, conn_net_cli_pkg_sn(pkg), NULL, NULL);
    if (require == NULL) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: require not exist, ignore!",
            conn_net_logic_sp_name(sp), conn_net_cli_pkg_sn(pkg));
        return -1;
    }

    data = logic_require_data_get_or_create(require, dp_req_meta(req), dp_req_size(req));
    if (data == NULL) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: create data fail!",
            conn_net_logic_sp_name(sp), conn_net_cli_pkg_sn(pkg));
        logic_require_set_error(require);
        return -1;
    }
    memcpy(logic_data_data(data), dp_req_data(req), dp_req_size(req));

    logic_require_set_done(require);
    return 0;
}

int conn_net_logic_sp_set_incoming_recv_at(conn_net_logic_sp_t sp, const char * incoming_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.incoming-recv-rsp", conn_net_logic_sp_name(sp));

    if (sp->m_incoming_recv_at) dp_rsp_free(sp->m_incoming_recv_at);

    sp->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(sp->m_app), name_buf);
    if (sp->m_incoming_recv_at == NULL) return -1;

    dp_rsp_set_processor(sp->m_incoming_recv_at, conn_net_logic_sp_incoming_recv, sp);

    if (dp_rsp_bind_string(sp->m_incoming_recv_at, incoming_recv_at, sp->m_em) != 0) {
        CPE_ERROR(
            sp->m_em, "%s: set incoming_recv_at: bind to %s fail!",
            conn_net_logic_sp_name(sp), incoming_recv_at);
        dp_rsp_free(sp->m_incoming_recv_at);
        sp->m_incoming_recv_at = NULL;
        return -1;
    }

    return 0;
}

int conn_net_logic_sp_send_request(
    conn_net_logic_sp_t sp,
    LPDRMETA meta, void const * data, size_t data_size,
    logic_require_t require)
{
    conn_net_cli_pkg_t pkg;
    dp_req_t body;

    body = conn_net_logic_sp_outgoing_buf(sp, data_size);
    if (body == NULL) {
        CPE_ERROR(sp->m_em, "%s: send_request: get body buf fail!", conn_net_logic_sp_name(sp));
        return -1;
    }

    pkg = conn_net_cli_pkg_find(body);
    assert(pkg);

    dp_req_set_buf(body, (void*)data, data_size);
    dp_req_set_size(body, data_size);
    dp_req_set_meta(body, meta);

    if (require) {
        conn_net_cli_pkg_set_sn(pkg, logic_require_id(require));
        if (logic_require_queue_add(sp->m_require_queue, logic_require_id(require), NULL, 0) != 0) {
            CPE_ERROR(sp->m_em, "%s: send_request: add require fail!", conn_net_logic_sp_name(sp));
            dp_req_set_buf(body, NULL, 0);
            dp_req_set_meta(body, NULL);
            return -1;
        }
    }

    if (dp_dispatch_by_string(sp->m_outgoing_dispatch_to, body, sp->m_em) != 0) {
        CPE_ERROR(
            sp->m_em, "%s: send_request: dispatch to %s fail!",
            conn_net_logic_sp_name(sp), cpe_hs_data(sp->m_outgoing_dispatch_to));
        dp_req_set_buf(body, NULL, 0);
        dp_req_set_meta(body, NULL);
        return -1;
    }

    dp_req_set_buf(body, NULL, 0);
    dp_req_set_meta(body, NULL);

    return 0;
}

dp_req_t conn_net_logic_sp_outgoing_buf(conn_net_logic_sp_t sp, size_t capacity) {
    if (sp->m_outgoing_pkg == NULL) {
        sp->m_outgoing_pkg = conn_net_cli_pkg_create(sp->m_cli);
        if (sp->m_outgoing_pkg == NULL) {
            CPE_ERROR(sp->m_em, "%s: crate outgoing pkg buf fail!", conn_net_logic_sp_name(sp));
            return NULL;
        }
    }

    if (sp->m_outgoing_body && dp_req_capacity(sp->m_outgoing_body) < capacity) {
        dp_req_free(sp->m_outgoing_body);
        sp->m_outgoing_body = NULL;
    }

    if (sp->m_outgoing_body == NULL) {
        sp->m_outgoing_body = dp_req_create(gd_app_dp_mgr(sp->m_app), capacity);
        if (sp->m_outgoing_body == NULL) {
            CPE_ERROR(sp->m_em, "%s: crate outgoing pkg buf fail!", conn_net_logic_sp_name(sp));
            return NULL;
        }

        dp_req_set_parent(sp->m_outgoing_body, conn_net_cli_pkg_to_dp_req(sp->m_outgoing_pkg));
    }

    conn_net_cli_pkg_init(sp->m_outgoing_pkg);
    dp_req_data_clear(sp->m_outgoing_body);

    return sp->m_outgoing_body;
}
