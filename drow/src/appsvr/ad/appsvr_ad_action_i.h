#ifndef APPSVR_AD_ACTION_I_H
#define APPSVR_AD_ACTION_I_H
#include "appsvr/ad/appsvr_ad_action.h"
#include "appsvr_ad_adapter_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_ad_action {
    appsvr_ad_module_t m_module;
    appsvr_ad_adapter_t m_adapter;
    TAILQ_ENTRY(appsvr_ad_action) m_next;
    struct cpe_hash_entry m_hh;
    appsvr_ad_request_list_t m_requests;
    const char * m_name;
    char m_name_buf[32];
};

uint32_t appsvr_ad_action_hash(appsvr_ad_action_t action);
int appsvr_ad_action_eq(appsvr_ad_action_t l, appsvr_ad_action_t r);
    
#ifdef __cplusplus
}
#endif

#endif
