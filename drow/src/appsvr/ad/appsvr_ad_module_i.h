#ifndef APPSVR_AD_MODULE_I_H
#define APPSVR_AD_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "appsvr/ad/appsvr_ad_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_ad_ui_state_monitor * appsvr_ad_ui_state_monitor_t;

typedef TAILQ_HEAD(appsvr_ad_adapter_list, appsvr_ad_adapter) appsvr_ad_adapter_list_t;
typedef TAILQ_HEAD(appsvr_ad_action_list, appsvr_ad_action) appsvr_ad_action_list_t;
typedef TAILQ_HEAD(appsvr_ad_request_list, appsvr_ad_request) appsvr_ad_request_list_t;

typedef TAILQ_HEAD(appsvr_ad_ui_state_monitor_list, appsvr_ad_ui_state_monitor) appsvr_ad_ui_state_monitor_list_t;
    
struct appsvr_ad_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;

    uint32_t m_adapter_count;
    appsvr_ad_adapter_list_t m_adapters;

    struct cpe_hash_table m_actions;

    uint32_t m_request_max_id;
    uint32_t m_request_count;
    appsvr_ad_request_list_t m_requests;
    appsvr_ad_request_list_t m_free_requests;

    /*state monitor*/
    plugin_ui_module_t m_ui_module;
    plugin_ui_state_t m_ui_cur_state;
    uint8_t m_ui_cur_state_processed;
    appsvr_ad_ui_state_monitor_list_t m_ui_state_monitors;
};

#ifdef __cplusplus
}
#endif

#endif
