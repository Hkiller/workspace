#ifndef APPSVR_AD_ADAPTER_I_H
#define APPSVR_AD_ADAPTER_I_H
#include "appsvr/ad/appsvr_ad_adapter.h"
#include "appsvr_ad_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_ad_adapter {
    appsvr_ad_module_t m_module;
    TAILQ_ENTRY(appsvr_ad_adapter) m_next;
    char m_name[32];
    void * m_ctx;
    appsvr_ad_start_fun_t m_req_start_fun;
    appsvr_ad_cancel_fun_t m_req_cancel_fun;
    appsvr_ad_action_list_t m_actions;
};
    
#ifdef __cplusplus
}
#endif

#endif
