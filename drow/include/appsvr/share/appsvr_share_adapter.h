#ifndef APPSVR_SHARE_ADAPTER_H
#define APPSVR_SHARE_ADAPTER_H
#include "appsvr_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*appsvr_share_commit_fun_t)(void * ctx, appsvr_share_request_t request);

struct appsvr_share_adapter_it {
    appsvr_share_adapter_t (*next)(struct appsvr_share_adapter_it * it);
    char m_data[64];
};

appsvr_share_adapter_t
appsvr_share_adapter_create(
    appsvr_share_module_t module, const char * name,
    void * ctx,
    appsvr_share_commit_fun_t commit_fun);

appsvr_share_adapter_t appsvr_share_adapter_first(appsvr_share_module_t module);

void appsvr_share_adapter_free(appsvr_share_adapter_t adapter);

void appsvr_share_module_adapters(appsvr_share_module_t module, appsvr_share_adapter_it_t adapter_it);

#define appsvr_share_adapter_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
