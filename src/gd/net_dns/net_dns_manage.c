#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_error.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "net_dns_manage_i.h"
#include "net_dns_task_i.h"

static void net_dns_manage_clear(nm_node_t node);
static void net_dns_manage_delay_clear(EV_P_ struct ev_timer *w, int revents);

struct nm_node_type s_nm_node_type_net_dns_manage = {
    "net_dns_manage",
    net_dns_manage_clear
};

net_dns_manage_t
net_dns_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    net_dns_manage_t mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct net_dns_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (net_dns_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    assert(mgr->m_ev_loop);
    
    ev_timer_init(&mgr->m_timer_event, net_dns_manage_delay_clear, 0.001, 0.);
    mgr->m_timer_event.data = mgr;
    
    TAILQ_INIT(&mgr->m_tasks);
    TAILQ_INIT(&mgr->m_deleting_tasks);

    nm_node_set_type(mgr_node, &s_nm_node_type_net_dns_manage);

    return mgr;
}

static void net_dns_manage_clear(nm_node_t node) {
    net_dns_manage_t mgr;
    mgr = (net_dns_manage_t)nm_node_data(node);

    if (ev_is_active(&mgr->m_timer_event)) {
        ev_timer_stop(mgr->m_ev_loop, &mgr->m_timer_event);
    }
    
    while(!TAILQ_EMPTY(&mgr->m_tasks)) {
        net_dns_task_free(TAILQ_FIRST(&mgr->m_tasks));
    }

    while(!TAILQ_EMPTY(&mgr->m_deleting_tasks)) {
        net_dns_task_free(TAILQ_FIRST(&mgr->m_deleting_tasks));
    }
}

void net_dns_manage_free(net_dns_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_net_dns_manage) return;
    nm_node_free(mgr_node);
}

gd_app_context_t net_dns_manage_app(net_dns_manage_t mgr) {
    return mgr->m_app;
}

net_dns_manage_t
net_dns_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_net_dns_manage) return NULL;
    return (net_dns_manage_t)nm_node_data(node);
}

net_dns_manage_t
net_dns_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "net_dns_manage";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_net_dns_manage) return NULL;
    return (net_dns_manage_t)nm_node_data(node);
}

const char * net_dns_manage_name(net_dns_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
net_dns_manage_name_hs(net_dns_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

static void net_dns_manage_delay_clear(EV_P_ struct ev_timer *w, int revents) {
    net_dns_manage_t mgr = w->data;

    while(!TAILQ_EMPTY(&mgr->m_deleting_tasks)) {
        net_dns_task_free(TAILQ_FIRST(&mgr->m_deleting_tasks));
    }
}
