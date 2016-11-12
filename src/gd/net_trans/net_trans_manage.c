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
#include "gd/net_trans/net_trans_detail.h"
#include "net_trans_manage_i.h"
#include "net_trans_task_i.h"

static void net_trans_manage_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_net_trans_manage = {
    "net_trans_manage",
    net_trans_manage_clear
};

net_trans_manage_t
net_trans_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct net_trans_manage * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct net_trans_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (net_trans_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
	mgr->m_multi_handle = NULL;
    mgr->m_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    mgr->m_timer_event.data = mgr;
    ev_timer_init(&mgr->m_timer_event, NULL, 0., 0.);
    mgr->m_still_running = 0;

    mgr->m_max_id = 0;

    mgr->m_cfg_dns_cache_timeout = 0;

    if (cpe_hash_table_init(
            &mgr->m_groups,
            alloc,
            (cpe_hash_fun_t) net_trans_group_hash,
            (cpe_hash_eq_t) net_trans_group_eq,
            CPE_HASH_OBJ2ENTRY(net_trans_group, m_hh_for_mgr),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_tasks,
            alloc,
            (cpe_hash_fun_t) net_trans_task_hash,
            (cpe_hash_eq_t) net_trans_task_eq,
            CPE_HASH_OBJ2ENTRY(net_trans_task, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_groups);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_net_trans_manage);

    return mgr;
}

static void net_trans_manage_clear(nm_node_t node) {
    net_trans_manage_t mgr;
    mgr = (net_trans_manage_t)nm_node_data(node);

    net_trans_group_free_all(mgr);

    if (mgr->m_multi_handle) {
        curl_multi_cleanup(mgr->m_multi_handle);
        mgr->m_multi_handle = NULL;
    }

    ev_timer_stop(mgr->m_loop, &mgr->m_timer_event);

    cpe_hash_table_fini(&mgr->m_groups);
    cpe_hash_table_fini(&mgr->m_tasks);
}

void net_trans_manage_free(net_trans_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_net_trans_manage) return;
    nm_node_free(mgr_node);
}

gd_app_context_t net_trans_manage_app(net_trans_manage_t mgr) {
    return mgr->m_app;
}

net_trans_manage_t
net_trans_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_net_trans_manage) return NULL;
    return (net_trans_manage_t)nm_node_data(node);
}

net_trans_manage_t
net_trans_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "net_trans_manage";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_net_trans_manage) return NULL;
    return (net_trans_manage_t)nm_node_data(node);
}

CURLM * net_trans_manage_handler(net_trans_manage_t mgr) {
    return mgr->m_multi_handle;
}

const char * net_trans_manage_name(net_trans_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
net_trans_manage_name_hs(net_trans_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

void net_trans_mgr_remove_tasks_by_commit_ctx(net_trans_manage_t mgr, void * ctx) {
    struct cpe_hash_it task_it;
    net_trans_task_t task;

    cpe_hash_it_init(&task_it, &mgr->m_tasks);

    task = cpe_hash_it_next(&task_it);
    while (task) {
        net_trans_task_t next = cpe_hash_it_next(&task_it);

        if (task->m_commit_ctx == ctx) {
            net_trans_task_free(task);
        }
        
        task = next;
    }
}

void net_trans_mgr_remove_tasks_by_commit_op(net_trans_manage_t mgr, net_trans_task_commit_op_t op, void * ctx) {
    struct cpe_hash_it task_it;
    net_trans_task_t task;

    cpe_hash_it_init(&task_it, &mgr->m_tasks);

    task = cpe_hash_it_next(&task_it);
    while (task) {
        net_trans_task_t next = cpe_hash_it_next(&task_it);

        if (task->m_commit_ctx == ctx && task->m_commit_op == op) {
            net_trans_task_free(task);
        }
        
        task = next;
    }
}
