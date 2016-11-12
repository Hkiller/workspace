#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/vnet/vnet_control_pkg.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_ops.h"

static void bpg_bind_manage_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_bind_manage = {
    "usf_bpg_bind_manage",
    bpg_bind_manage_clear
};

bpg_bind_manage_t
bpg_bind_manage_create(
    gd_app_context_t app,
    mem_allocrator_t alloc,
    const char * name,
    bpg_pkg_manage_t pkg_mgr,
    error_monitor_t em)
{
    struct bpg_bind_manage * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_bind_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_bind_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_pkg_manage = pkg_mgr;
    mgr->m_cmd_kickoff = 0;

    mgr->m_data_pkg = NULL;
    mgr->m_control_pkg = NULL;

    mgr->m_incoming_recv_at = NULL;
	mgr->m_incoming_send_to = NULL;
    mgr->m_outgoing_recv_at = NULL;
	mgr->m_outgoing_send_to = NULL;

    if (cpe_hash_table_init(
            &mgr->m_cliensts,
            alloc,
            (cpe_hash_fun_t) bpg_bind_binding_client_id_hash,
            (cpe_hash_eq_t) bpg_bind_binding_client_id_cmp,
            CPE_HASH_OBJ2ENTRY(bpg_bind_binding, m_hh_client),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_connections,
            alloc,
            (cpe_hash_fun_t) bpg_bind_binding_connection_id_hash,
            (cpe_hash_eq_t) bpg_bind_binding_connection_id_cmp,
            CPE_HASH_OBJ2ENTRY(bpg_bind_binding, m_hh_connection),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_cliensts);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_bind_manage);

    return mgr;
}

static void bpg_bind_manage_clear(nm_node_t node) {
    bpg_bind_manage_t mgr;
    mgr = (bpg_bind_manage_t)nm_node_data(node);

    cpe_hash_table_fini(&mgr->m_cliensts);
    cpe_hash_table_fini(&mgr->m_connections);

    if (mgr->m_data_pkg) {
        dp_req_free(mgr->m_data_pkg);
        mgr->m_data_pkg = NULL;
    }

    if (mgr->m_control_pkg) {
        vnet_control_pkg_free(mgr->m_control_pkg);
        mgr->m_control_pkg = NULL;
    }

    if (mgr->m_incoming_recv_at) {
        dp_rsp_free(mgr->m_incoming_recv_at);
        mgr->m_incoming_recv_at = NULL;
    }

    if (mgr->m_incoming_send_to) {
        bpg_pkg_dsp_free(mgr->m_incoming_send_to);
        mgr->m_incoming_send_to = NULL;
    }

    if (mgr->m_outgoing_recv_at) {
        dp_rsp_free(mgr->m_outgoing_recv_at);
        mgr->m_outgoing_recv_at = NULL;
    }

    if (mgr->m_outgoing_send_to) {
        bpg_pkg_dsp_free(mgr->m_outgoing_send_to);
        mgr->m_outgoing_send_to = NULL;
    }
}

gd_app_context_t bpg_bind_manage_app(bpg_bind_manage_t mgr) {
    return mgr->m_app;
}

void bpg_bind_manage_free(bpg_bind_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_bind_manage) return;
    nm_node_free(mgr_node);
}

bpg_bind_manage_t
bpg_bind_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_bind_manage) return NULL;
    return (bpg_bind_manage_t)nm_node_data(node);
}

bpg_bind_manage_t
bpg_bind_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_bind_manage) return NULL;
    return (bpg_bind_manage_t)nm_node_data(node);
}

const char * bpg_bind_manage_name(bpg_bind_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
bpg_bind_manage_name_hs(bpg_bind_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int bpg_bind_manage_set_incoming_send_to(bpg_bind_manage_t mgr, cfg_t cfg) {
    if (mgr->m_incoming_send_to != NULL) {
        bpg_pkg_dsp_free(mgr->m_incoming_send_to);
        mgr->m_incoming_send_to = NULL;
    }

    mgr->m_incoming_send_to = bpg_pkg_dsp_create(mgr->m_alloc);
    if (mgr->m_incoming_send_to == NULL) return -1;

    if (bpg_pkg_dsp_load(mgr->m_incoming_send_to, cfg, mgr->m_em) != 0) return -1;

    return 0;
}

int bpg_bind_manage_set_incoming_recv_at(bpg_bind_manage_t mgr, const char * name) {
    char sp_name_buf[128];

    if (mgr->m_incoming_recv_at != NULL) {
        dp_rsp_free(mgr->m_incoming_recv_at);
        mgr->m_incoming_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.incoming.recv.sp", bpg_bind_manage_name(mgr));
    mgr->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(mgr->m_app), sp_name_buf);
    if (mgr->m_incoming_recv_at == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: bpg_bind_manage_set_incoming_recv_at: create rsp fail!",
            bpg_bind_manage_name(mgr));
        return -1;
    }
    dp_rsp_set_processor(mgr->m_incoming_recv_at, bpg_bind_manage_incoming_rsp, mgr);

    if (dp_rsp_bind_string(mgr->m_incoming_recv_at, name, mgr->m_em) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: bpg_bind_manage_set_incoming_recv_at: bind rsp to %s fail!",
            bpg_bind_manage_name(mgr), name);
        dp_rsp_free(mgr->m_incoming_recv_at);
        mgr->m_incoming_recv_at = NULL;
        return -1;
    }

    return 0;
}

int bpg_bind_manage_set_outgoing_send_to(bpg_bind_manage_t mgr, cfg_t cfg) {
    if (mgr->m_outgoing_send_to != NULL) {
        bpg_pkg_dsp_free(mgr->m_outgoing_send_to);
        mgr->m_outgoing_send_to = NULL;
    }

    mgr->m_outgoing_send_to = bpg_pkg_dsp_create(mgr->m_alloc);
    if (mgr->m_outgoing_send_to == NULL) return -1;

    if (bpg_pkg_dsp_load(mgr->m_outgoing_send_to, cfg, mgr->m_em) != 0) return -1;

    return 0;
}

int bpg_bind_manage_set_outgoing_recv_at(bpg_bind_manage_t mgr, const char * name) {
    char sp_name_buf[128];

    if (mgr->m_outgoing_recv_at != NULL) {
        dp_rsp_free(mgr->m_outgoing_recv_at);
        mgr->m_outgoing_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.outgoing.recv.sp", bpg_bind_manage_name(mgr));
    mgr->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(mgr->m_app), sp_name_buf);
    if (mgr->m_outgoing_recv_at == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: bpg_bind_manage_set_outgoing_recv_at: create rsp fail!",
            bpg_bind_manage_name(mgr));
        return -1;
    }
    dp_rsp_set_processor(mgr->m_outgoing_recv_at, bpg_bind_manage_outgoing_rsp, mgr);

    if (dp_rsp_bind_string(mgr->m_outgoing_recv_at, name, mgr->m_em) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: bpg_bind_manage_set_outgoing_recv_at: bind rsp to %s fail!",
            bpg_bind_manage_name(mgr), name);
        dp_rsp_free(mgr->m_outgoing_recv_at);
        mgr->m_outgoing_recv_at = NULL;
        return -1;
    }

    return 0;
}

bpg_pkg_manage_t bpg_bind_manage_pkg_manage(bpg_bind_manage_t mgr) {
    return mgr->m_pkg_manage;
}

dp_req_t bpg_bind_manage_data_pkg(bpg_bind_manage_t mgr) {
    if (mgr->m_data_pkg == NULL) {
        mgr->m_data_pkg = bpg_pkg_create_with_body(mgr->m_pkg_manage, 1024);
    }

    return mgr->m_data_pkg;
}

vnet_control_pkg_t bpg_bind_manage_control_pkg(bpg_bind_manage_t mgr) {
    if (mgr->m_control_pkg == NULL) {
        mgr->m_control_pkg = vnet_control_pkg_create(mgr->m_app, 0);
    }

    return mgr->m_control_pkg;
}
