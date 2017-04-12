#ifndef USF_BPG_NET_AGENT_TCP_H
#define USF_BPG_NET_AGENT_TCP_H
#include "cpe/tl/tl_types.h"
#include "bpg_net_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_net_agent_t
bpg_net_agent_create(
    gd_app_context_t app,
    bpg_pkg_manage_t pkg_manage,
    const char * name,
    const char * ip,
    short port,
    int acceptQueueSize,
    mem_allocrator_t alloc,
    error_monitor_t em);

void bpg_net_agent_free(bpg_net_agent_t svr);

bpg_net_agent_t
bpg_net_agent_find(gd_app_context_t app, cpe_hash_string_t name);

bpg_net_agent_t
bpg_net_agent_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t bpg_net_agent_app(bpg_net_agent_t mgr);
const char * bpg_net_agent_name(bpg_net_agent_t mgr);
cpe_hash_string_t bpg_net_agent_name_hs(bpg_net_agent_t mgr);

void bpg_net_agent_set_conn_timeout(bpg_net_agent_t agent, tl_time_span_t span);
tl_time_span_t bpg_net_agent_conn_timeout(bpg_net_agent_t agent);

int bpg_net_agent_set_dispatch_to(bpg_net_agent_t agent, const char * dispatch_to);
short bpg_net_agent_port(bpg_net_agent_t svr);

#ifdef __cplusplus
}
#endif

#endif
