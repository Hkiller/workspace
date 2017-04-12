#ifndef APPSVR_AD_ADAPTER_H
#define APPSVR_AD_ADAPTER_H
#include "appsvr_ad_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*appsvr_ad_cancel_fun_t)(void * ctx, appsvr_ad_request_t request, appsvr_ad_action_t action);
typedef int (*appsvr_ad_start_fun_t)(void * ctx, appsvr_ad_request_t request, appsvr_ad_action_t action);

appsvr_ad_adapter_t
appsvr_ad_adapter_create(
    appsvr_ad_module_t module, const char * name,
    void * ctx,
    appsvr_ad_start_fun_t req_start_fun,
    appsvr_ad_cancel_fun_t req_cancel_fun);
    
void appsvr_ad_adapter_free(appsvr_ad_adapter_t adapter);
    
#ifdef __cplusplus
}
#endif

#endif
