#ifndef GD_NET_DNS_MANAGE_H
#define GD_NET_DNS_MANAGE_H
#include "net_dns_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_dns_manage_t
net_dns_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void net_dns_manage_free(net_dns_manage_t mgr);

net_dns_manage_t
net_dns_manage_find(gd_app_context_t app, cpe_hash_string_t name);

net_dns_manage_t
net_dns_manage_find_nc(gd_app_context_t app, const char * name);

net_dns_manage_t
net_dns_manage_default(gd_app_context_t app);

gd_app_context_t net_dns_manage_app(net_dns_manage_t mgr);
const char * net_dns_manage_name(net_dns_manage_t mgr);
cpe_hash_string_t net_dns_manage_name_hs(net_dns_manage_t mgr);
    
#ifdef __cplusplus
}
#endif

#endif
