#ifndef CPE_NET_CONNECTOR_H
#define CPE_NET_CONNECTOR_H
#include "cpe/utils/memory.h"
#include "net_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_connector_t
net_connector_create(
    net_mgr_t nmgr,
    const char * name,
    const char * ip,
    short port);

net_connector_t
net_connector_create_with_ep(
    net_mgr_t nmgr,
    const char * name,
    const char * ip,
    short port);

int net_connector_set_address(
    net_connector_t connector,
    const char * ip,
    short port);

void net_connector_free(net_connector_t connector);

const char * net_connector_state_str(net_connector_state_t state);

net_connector_t
net_connector_find(net_mgr_t nmgr, const char * name);

const char * net_connector_name(net_connector_t connector);
net_connector_state_t net_connector_state(net_connector_t connector);

int net_connector_bind(net_connector_t connector, net_ep_t ep);
int net_connector_unbind(net_connector_t connector);

int net_connector_enable(net_connector_t connector);
void net_connector_disable(net_connector_t connector);

int net_connector_add_monitor(
    net_connector_t connector,
    net_connector_state_monitor_fun_t fun, void * ctx);

int net_connector_remove_monitor(
    net_connector_t connector,
    net_connector_state_monitor_fun_t fun, void * ctx);

void net_connector_set_reconnect_span_ms(
    net_connector_t connector, uint64_t span_ms);

net_ep_t net_connector_ep(net_connector_t connector);

const char * net_connector_ip(net_connector_t connector);
short net_connector_port(net_connector_t connector);

#ifdef __cplusplus
}
#endif

#endif

