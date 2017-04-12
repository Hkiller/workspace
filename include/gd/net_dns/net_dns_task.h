#ifndef GD_NET_DNS_TASK_H
#define GD_NET_DNS_TASK_H
#include "cpe/utils/buffer.h"
#include "net_dns_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_dns_task_t net_dns_ares_gethostbyname(
    net_dns_manage_t manager,
    const char * hostname,
    net_dns_process_fun_t process_fun, void * process_ctx);
    
void net_dns_task_free(net_dns_task_t task);

void net_dns_task_free_by_ctx(net_dns_manage_t manager, void * process_ctx);

#ifdef __cplusplus
}
#endif

#endif
