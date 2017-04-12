#ifndef APPSVR_AD_ACTION_H
#define APPSVR_AD_ACTION_H
#include "appsvr_ad_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_ad_action_t
appsvr_ad_action_create(appsvr_ad_adapter_t adapter, const char * name, size_t capacity);
void appsvr_ad_action_free(appsvr_ad_action_t action);

void * appsvr_ad_action_data(appsvr_ad_action_t action);

appsvr_ad_action_t
appsvr_ad_action_find(appsvr_ad_module_t module, const char * name);
    
#ifdef __cplusplus
}
#endif

#endif
