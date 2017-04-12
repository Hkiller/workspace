#ifndef APPSVR_AD_TYPES_H
#define APPSVR_AD_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "cpe/cfg/cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_ad_module * appsvr_ad_module_t;
typedef struct appsvr_ad_request * appsvr_ad_request_t;
typedef struct appsvr_ad_adapter * appsvr_ad_adapter_t;
typedef struct appsvr_ad_action * appsvr_ad_action_t;

typedef enum appsvr_ad_result {
    appsvr_ad_start_skip,
    appsvr_ad_start_fail,
    appsvr_ad_start_cancel,
    appsvr_ad_start_started,
    appsvr_ad_start_success,
} appsvr_ad_result_t;
        
typedef void (*appsvr_ad_resonse_fun_t)(void * ctx, void * arg, uint32_t req_id, appsvr_ad_result_t result);

typedef appsvr_ad_adapter_t
(*appsvr_ad_adapter_creation_fun_t)(
    appsvr_ad_module_t ad_module, cfg_t cfg, mem_allocrator_t alloc, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
