#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/net/net_manage.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "net_internal_ops.h"

net_mgr_t
net_mgr_create(mem_allocrator_t alloc, error_monitor_t em) {
    net_mgr_t nmgr;

    nmgr = (net_mgr_t)mem_alloc(alloc, sizeof(struct net_mgr));
    if (nmgr == NULL) return NULL;
    bzero(nmgr, sizeof(struct net_mgr));

    if (cpe_range_mgr_init(&nmgr->m_ep_ids, alloc) != 0) {
        CPE_ERROR(em, "init ep ids fail!");
        mem_free(alloc, nmgr);
        return NULL;
    }

    nmgr->m_ev_loop = ev_loop_new(EVFLAG_AUTO);
    if (!nmgr->m_ev_loop) {
        CPE_ERROR(em, "net_mgr_create: create event loop fail!");
        cpe_range_mgr_fini(&nmgr->m_ep_ids);
        mem_free(alloc, nmgr);
        return NULL;
    }

    nmgr->m_alloc = alloc;
    nmgr->m_em = em;
    nmgr->m_debug = 0;
    nmgr->m_ep_page_capacity = 0;
    nmgr->m_ep_pages = NULL;

    TAILQ_INIT(&nmgr->m_chanels);

    if (cpe_hash_table_init(
            &nmgr->m_listeners,
            alloc,
            (cpe_hash_fun_t)net_listener_hash,
            (cpe_hash_eq_t)net_listener_cmp,
            CPE_HASH_OBJ2ENTRY(net_listener, m_hh),
            256) != 0)
    {
        CPE_ERROR(em, "net_mgr_create: init listener hash list fail!");
        cpe_range_mgr_fini(&nmgr->m_ep_ids);
        mem_free(alloc, nmgr);
        return NULL;
    }

    if (cpe_hash_table_init(
            &nmgr->m_connectors,
            alloc,
            (cpe_hash_fun_t)net_connector_hash,
            (cpe_hash_eq_t)net_connector_cmp,
            CPE_HASH_OBJ2ENTRY(net_connector, m_hh),
            256) != 0)
    {
        CPE_ERROR(em, "net_mgr_create: init connector hash list fail!");
        cpe_hash_table_fini(&nmgr->m_listeners);
        cpe_range_mgr_fini(&nmgr->m_ep_ids);
        mem_free(alloc, nmgr);
        return NULL;
    }

    return nmgr;
}

void net_mgr_free(net_mgr_t nmgr) {
    net_chanel_t chanel;

    assert(nmgr);

    /*free connectors*/
    net_connectors_free(nmgr);
    cpe_hash_table_fini(&nmgr->m_connectors);

    /*free listeners*/
    net_listeners_free(nmgr);
    cpe_hash_table_fini(&nmgr->m_listeners);

    /*free eps*/
    net_ep_pages_free(nmgr);
    cpe_range_mgr_fini(&nmgr->m_ep_ids);

    /*free event loop*/
    ev_loop_destroy(nmgr->m_ev_loop);
    nmgr->m_ev_loop = NULL;

    /*TODO*/
    while((chanel = TAILQ_FIRST(&nmgr->m_chanels))) {
        net_chanel_free(chanel);
    }

    mem_free(nmgr->m_alloc, nmgr);
}

error_monitor_t net_mgr_em(net_mgr_t nmgr) {
    return nmgr->m_em;
}

int net_mgr_debug(net_mgr_t nmgr) {
    return nmgr->m_debug;
}

void net_mgr_set_debug(net_mgr_t nmgr, int debug) {
    nmgr->m_debug = debug;
}

struct ev_loop * net_mgr_ev_loop(net_mgr_t nmgr) {
    return nmgr->m_ev_loop;
}
