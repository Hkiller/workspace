#ifndef GD_NET_TRANS_INTERNAL_TYPES_H
#define GD_NET_TRANS_INTERNAL_TYPES_H
#include "curl/curl.h"
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/net_trans/net_trans_manage.h"

typedef TAILQ_HEAD(net_trans_task_list, net_trans_task) net_trans_task_list_t;
 
struct net_trans_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    struct ev_loop * m_loop;
    struct ev_timer m_timer_event;
	CURLM * m_multi_handle;
    int m_still_running;

    int m_cfg_dns_cache_timeout;

    uint32_t m_max_id;
    struct cpe_hash_table m_groups;
    struct cpe_hash_table m_tasks;

    int m_debug;
};

int net_trans_mult_handler_init(net_trans_manage_t svr);

#endif

