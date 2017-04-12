#ifndef CPE_NET_LISTENER_H
#define CPE_NET_LISTENER_H
#include "cpe/utils/memory.h"
#include "net_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_listener_t
net_listener_create(
    net_mgr_t nmgr,
    const char * name,
    const char * ip,
    short port,
    int acceptQueueSize,
    net_accept_fun_t acceptor,
    void * acceptor_ctx);

void net_listener_free(net_listener_t listener);

net_listener_t
net_listener_find(net_mgr_t nmgr, const char * name);

const char * net_listener_name(net_listener_t listener);
short net_listener_using_port(net_listener_t listener);

#ifdef __cplusplus
}
#endif

#endif

