#ifndef GD_NET_INTERNAL_TYPES_H
#define GD_NET_INTERNAL_TYPES_H

#include "ev.h"

#include "cpe/pal/pal_queue.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/range.h"
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(net_chanel_list, net_chanel) net_chanel_list_t;

#define GD_NET_EP_INVALID_ID ((net_ep_id_t)-1)
#define GD_NET_EP_COUNT_PER_PAGE (256)
#define GD_NET_EP_PAGE_INC (16)
struct net_ep_page;

struct net_mgr {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    struct cpe_range_mgr m_ep_ids;
    size_t m_ep_page_capacity;
    struct net_ep_page * * m_ep_pages;

    net_chanel_list_t m_chanels;

    struct cpe_hash_table m_listeners;
    struct cpe_hash_table m_connectors;

    struct ev_loop * m_ev_loop;
};

struct net_listener {
    net_mgr_t m_mgr;
    char const * m_name;
    char m_addr[16]; /*sizeof(sockaddr)*/
    int m_acceptQueueSize;
    net_accept_fun_t m_acceptor_fun;
    void * m_acceptor_ctx;
    int m_fd;
    struct ev_io m_watcher;

    struct cpe_hash_entry m_hh;
};

struct net_connector_monitor {
    net_connector_state_monitor_fun_t m_monitor_fun;
    void * m_monitor_ctx;
    struct net_connector_monitor * m_next;
};

struct net_connector {
    net_mgr_t m_mgr;
    char const * m_name;
    char m_addr[16]; /*sizeof(sockaddr)*/
    char m_ip[16];
    short m_port;
    net_ep_t m_ep;
    int8_t m_processing;
    int8_t m_deleted;
    int8_t m_unregisted;
    net_connector_state_t m_state;
    double m_reconnect_span;
    struct net_connector_monitor * m_monitors;
    struct ev_timer m_timer;
    struct cpe_hash_entry m_hh;
};

struct net_chanel_type {
    const char * name;
    net_chanel_type_id_t id;
    ssize_t (*read_from_net)(net_chanel_t chanel, int fd);
    ssize_t (*write_to_net)(net_chanel_t chanel, int fd);
    int (*read_from_buf)(net_chanel_t chanel, const void * buf, size_t size);
    ssize_t (*write_to_buf)(net_chanel_t chanel, void * buf, size_t capacity);
    void * (*peek)(net_chanel_t chanel, void * buf, size_t size);
    void (*erase)(net_chanel_t chanel, size_t size);
    size_t (*data_size)(net_chanel_t chanel);
    void (*destory)(net_chanel_t chanel);
};

#define NET_CHANEL_COMMON                       \
    net_mgr_t m_mgr;                            \
    struct net_chanel_type * m_type;            \
    net_chanel_state_t m_state;                 \
    TAILQ_ENTRY(net_chanel) m_next

struct net_chanel {
    NET_CHANEL_COMMON;
};

struct net_chanel_queue {
    NET_CHANEL_COMMON;
    char * m_buf;
    size_t m_size;
    size_t m_capacity;
    void (*m_destory_fun)(net_chanel_t chanel, void * ctx);
    void * m_destory_ctx;

};

struct net_ep {
    net_ep_id_t m_id;
    net_mgr_t m_mgr;
    net_chanel_t m_chanel_r;
    net_chanel_t m_chanel_w;
    struct net_connector * m_connector;
    net_process_fun_t m_process_fun;
    void * m_process_ctx;
    int m_fd;
    int8_t m_processing;
    int8_t m_deleted;
    struct ev_io m_watcher;
    struct ev_timer m_timer;
	enum net_status m_status;
};

struct net_ep_page {
    struct net_ep m_eps[GD_NET_EP_COUNT_PER_PAGE];
};

#ifdef __cplusplus
}
#endif

#endif
