#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_net/bpg_net_client.h"
#include "bpg_net_internal_ops.h"

struct nm_node_type s_nm_node_type_bpg_net_client;

bpg_net_client_t
bpg_net_client_create(
    gd_app_context_t app,
    bpg_pkg_manage_t pkg_manage,
    logic_manage_t logic_manage,
    const char * name,
    const char * ip,
    short port,
    size_t read_chanel_size,
    size_t write_chanel_size,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    bpg_net_client_t mgr;
    nm_node_t mgr_node;
    char name_buf[128];

    assert(name);
    assert(ip);

    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_net_client));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_net_client_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_pkg_manage = pkg_manage;
    mgr->m_logic_mgr = logic_manage;
    mgr->m_em = em;
    mgr->m_req_max_size = 4 * 1024;
    mgr->m_req_buf = NULL;
    mgr->m_runing_require_capacity = 0;
    mgr->m_runing_require_count = 0;
    mgr->m_runing_require_op_count = 0;
    mgr->m_runing_require_check_span = 20000;
    mgr->m_runing_requires = NULL;
    mgr->m_debug = 0;

    mem_buffer_init(&mgr->m_send_encode_buf, alloc);
    mem_buffer_init(&mgr->m_dump_buffer, alloc);

    mgr->m_rsp_dsp = bpg_pkg_dsp_create(alloc);
    if (mgr->m_rsp_dsp == NULL) {
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    snprintf(name_buf, sizeof(name_buf), "%s.send-rsp", name);
    mgr->m_send_rsp = dp_rsp_create(gd_app_dp_mgr(app), name_buf);
    if (mgr->m_send_rsp == NULL) {
        bpg_pkg_dsp_free(mgr->m_rsp_dsp);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }
    dp_rsp_set_processor(mgr->m_send_rsp, bpg_net_client_send, mgr);

    mgr->m_connector =
        net_connector_create_with_ep(
            gd_app_net_mgr(app),
            name,
            ip,
            port);
    if (mgr->m_connector == NULL) {
        dp_rsp_free(mgr->m_send_rsp);
        bpg_pkg_dsp_free(mgr->m_rsp_dsp);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (bpg_net_client_ep_init(mgr, net_connector_ep(mgr->m_connector), read_chanel_size, write_chanel_size) != 0) {
        net_connector_free(mgr->m_connector);
        dp_rsp_free(mgr->m_send_rsp);
        bpg_pkg_dsp_free(mgr->m_rsp_dsp);
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_send_encode_buf);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_net_client);

    return mgr;
}

static void bpg_net_client_clear(nm_node_t node) {
    bpg_net_client_t mgr;
    mgr = (bpg_net_client_t)nm_node_data(node);

    net_connector_free(mgr->m_connector);

    if (mgr->m_req_buf) {
        dp_req_free(mgr->m_req_buf);
        mgr->m_req_buf = NULL;
    }

    if (mgr->m_rsp_dsp) {
        bpg_pkg_dsp_free(mgr->m_rsp_dsp);
        mgr->m_rsp_dsp = NULL;
    }

    if (mgr->m_send_rsp) {
        dp_rsp_free(mgr->m_send_rsp);
        mgr->m_send_rsp = NULL;
    }

    if (mgr->m_runing_requires) {
        mem_free(mgr->m_alloc, mgr->m_runing_requires);
        mgr->m_runing_requires = NULL;
    }

    mem_buffer_clear(&mgr->m_dump_buffer);
    mem_buffer_clear(&mgr->m_send_encode_buf);
}

void bpg_net_client_free(bpg_net_client_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_net_client) return;
    nm_node_free(mgr_node);
}

bpg_net_client_t
bpg_net_client_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_net_client) return NULL;
    return (bpg_net_client_t)nm_node_data(node);
}

bpg_net_client_t
bpg_net_client_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_net_client) return NULL;
    return (bpg_net_client_t)nm_node_data(node);
}

gd_app_context_t bpg_net_client_app(bpg_net_client_t mgr) {
    return mgr->m_app;
}

const char * bpg_net_client_name(bpg_net_client_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
bpg_net_client_name_hs(bpg_net_client_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

dp_req_t
bpg_net_client_req_buf(bpg_net_client_t mgr) {
    if (mgr->m_req_buf) {
        if (dp_req_capacity(mgr->m_req_buf) < mgr->m_req_max_size) {
            dp_req_free(mgr->m_req_buf);
            mgr->m_req_buf = NULL;
        }
    }

    if (mgr->m_req_buf == NULL) {
        mgr->m_req_buf = bpg_pkg_create_with_body(mgr->m_pkg_manage, mgr->m_req_max_size);
    }

    return mgr->m_req_buf;
}

net_connector_t bpg_net_client_connector(bpg_net_client_t mgr) {
    return mgr->m_connector;
}

bpg_pkg_manage_t bpg_net_client_pkg_manage(bpg_net_client_t req) {
    return req->m_pkg_manage;
}

void bpg_net_client_set_reconnect_span_ms(bpg_net_client_t client, uint32_t span_ms) {
    net_connector_set_reconnect_span_ms(client->m_connector, span_ms);
}

static void bpg_net_client_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_net_client = {
    "usf_bpg_net_client",
    bpg_net_client_clear
};
