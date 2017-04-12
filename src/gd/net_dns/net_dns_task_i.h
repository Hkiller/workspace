#ifndef GD_NET_DNS_TASK_I_H
#define GD_NET_DNS_TASK_I_H
#include "gd/net_dns/net_dns_task.h"
#include "net_dns_manage_i.h"

struct net_dns_task {
    net_dns_manage_t m_mgr;
    TAILQ_ENTRY(net_dns_task) m_next;
    net_dns_process_fun_t m_process_fun;
    void * m_process_ctx;
    uint8_t m_is_processing;
    uint8_t m_is_destoried;
    
    ares_channel m_ares_chanel;
    struct ev_io m_watcher;
    char m_buf[32];
};

void net_dns_socket_callback(void *data, ares_socket_t socket_fd, int readable, int writable);

#endif


