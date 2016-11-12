#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/net/net_listener.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_net/bpg_net_agent.h"
#include "bpg_net_internal_ops.h"

struct nm_node_type s_nm_node_type_bpg_net_agent;

bpg_net_agent_t
bpg_net_agent_create(
    gd_app_context_t app,
    bpg_pkg_manage_t pkg_manage,
    const char * name,
    const char * ip,
    short port,
    int acceptQueueSize,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    bpg_net_agent_t mgr;
    nm_node_t mgr_node;
    char name_buf[128];

    assert(app);
    assert(pkg_manage);
    assert(name);
    assert(ip);

    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_net_agent));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_net_agent_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_pkg_manage = pkg_manage;
    mgr->m_em = em;
    mgr->m_req_max_size = 4 * 1024;
    mgr->m_req_buf = NULL;
    mgr->m_debug = 0;
    mgr->m_read_chanel_size = 2048;
    mgr->m_write_chanel_size = 2048;
    mgr->m_conn_timeout = 0;
    mgr->m_dispatch_to = NULL;

    mem_buffer_init(&mgr->m_dump_buffer, alloc);
    mem_buffer_init(&mgr->m_rsp_buf, alloc);

    snprintf(name_buf, sizeof(name_buf), "%s.reply-rsp", name);
    mgr->m_reply_rsp = dp_rsp_create(gd_app_dp_mgr(app), name_buf);
    if (mgr->m_reply_rsp == NULL) {
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_rsp_buf);
        nm_node_free(mgr_node);
        return NULL;
    }
    dp_rsp_set_processor(mgr->m_reply_rsp, bpg_net_agent_reply, mgr);

    mgr->m_listener =
        net_listener_create(
            gd_app_net_mgr(app),
            name,
            ip,
            port,
            acceptQueueSize,
            bpg_net_agent_accept,
            mgr);
    if (mgr->m_listener == NULL) {
        mem_buffer_clear(&mgr->m_dump_buffer);
        mem_buffer_clear(&mgr->m_rsp_buf);
        dp_rsp_free(mgr->m_reply_rsp);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_net_agent);

    return mgr;
}

static void bpg_net_agent_clear(nm_node_t node) {
    bpg_net_agent_t mgr;
    mgr = (bpg_net_agent_t)nm_node_data(node);

    mem_buffer_clear(&mgr->m_dump_buffer);
    mem_buffer_clear(&mgr->m_rsp_buf);

    dp_rsp_free(mgr->m_reply_rsp);
    mgr->m_reply_rsp = NULL;

    if (mgr->m_req_buf) {
        dp_req_free(mgr->m_req_buf);
        mgr->m_req_buf = NULL;
    }

    if (mgr->m_dispatch_to) {
        mem_free(mgr->m_alloc, mgr->m_dispatch_to);
        mgr->m_dispatch_to = NULL;
    }

    net_listener_free(mgr->m_listener);
}

void bpg_net_agent_free(bpg_net_agent_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_net_agent) return;
    nm_node_free(mgr_node);
}

bpg_net_agent_t
bpg_net_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_net_agent) return NULL;
    return (bpg_net_agent_t)nm_node_data(node);
}

bpg_net_agent_t
bpg_net_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_net_agent) return NULL;
    return (bpg_net_agent_t)nm_node_data(node);
}

gd_app_context_t bpg_net_agent_app(bpg_net_agent_t mgr) {
    return mgr->m_app;
}

const char * bpg_net_agent_name(bpg_net_agent_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
bpg_net_agent_name_hs(bpg_net_agent_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

short bpg_net_agent_port(bpg_net_agent_t svr) {
    return net_listener_using_port(svr->m_listener);
}

int bpg_net_agent_set_dispatch_to(bpg_net_agent_t agent, const char * dispatch_to) {
    size_t name_len;

    if (agent->m_dispatch_to) {
        mem_free(agent->m_alloc, agent->m_dispatch_to);
        agent->m_dispatch_to = NULL;
    }

    if (dispatch_to) {
        name_len = cpe_hs_len_to_binary_len(strlen(dispatch_to));
        agent->m_dispatch_to = (cpe_hash_string_t)mem_alloc(agent->m_alloc, name_len);
        if (agent->m_dispatch_to == NULL) return -1;

        cpe_hs_init(agent->m_dispatch_to, name_len, dispatch_to);
    }

    return 0;
}

void bpg_net_agent_set_conn_timeout(bpg_net_agent_t agent, tl_time_span_t span) {
    agent->m_conn_timeout = span;
}

tl_time_span_t bpg_net_agent_conn_timeout(bpg_net_agent_t agent) {
    return agent->m_conn_timeout;
}

dp_req_t
bpg_net_agent_req_buf(bpg_net_agent_t mgr) {
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

static void bpg_net_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_net_agent = {
    "usf_bpg_net_agent",
    bpg_net_agent_clear
};
