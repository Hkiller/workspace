#ifndef GD_NET_DNS_INTERNAL_TYPES_H
#define GD_NET_DNS_INTERNAL_TYPES_H
#include "ares.h"
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "gd/net_dns/net_dns_manage.h"

typedef TAILQ_HEAD(net_dns_task_list, net_dns_task) net_dns_task_list_t;
 
struct net_dns_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    struct ev_loop * m_ev_loop;
    struct ev_timer m_timer_event;

    net_dns_task_list_t m_tasks;
    net_dns_task_list_t m_deleting_tasks;
};

#endif

