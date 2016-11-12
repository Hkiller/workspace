#ifndef GD_NET_DNS_TYPES_H
#define GD_NET_DNS_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct net_dns_manage * net_dns_manage_t;
typedef struct net_dns_task * net_dns_task_t;

typedef void (*net_dns_process_fun_t)(void * ctx, const char * ip);
    
#ifdef __cplusplus
}
#endif

#endif
