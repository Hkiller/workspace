#ifndef APPSVR_AD_REQUEST_H
#define APPSVR_AD_REQUEST_H
#include "appsvr_ad_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_ad_request_t appsvr_ad_request_find_by_id(appsvr_ad_module_t module, uint32_t id);

uint32_t appsvr_ad_request_id(appsvr_ad_request_t request);

int appsvr_ad_request_set_result(appsvr_ad_request_t request, appsvr_ad_result_t result);
    
#ifdef __cplusplus
}
#endif

#endif
