#ifndef APPSVR_AD_REQUEST_I_H
#define APPSVR_AD_REQUEST_I_H
#include "appsvr/ad/appsvr_ad_request.h"
#include "appsvr_ad_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_ad_request {
    appsvr_ad_module_t m_module;
    TAILQ_ENTRY(appsvr_ad_request) m_next_for_module;
    appsvr_ad_action_t m_action;
    TAILQ_ENTRY(appsvr_ad_request) m_next_for_action;
    uint32_t m_id;
    void * m_ctx;
    appsvr_ad_resonse_fun_t m_response_fun;
    void * m_arg;
    void (*m_arg_free)(void * ctx);
    uint8_t m_in_process;
    appsvr_ad_result_t m_result;
};

appsvr_ad_request_t
appsvr_ad_request_create(
    appsvr_ad_module_t module, appsvr_ad_action_t action,
    void * ctx,
    appsvr_ad_resonse_fun_t response_fun,
    void * arg,
    void (*arg_free)(void * ctx));
    
void appsvr_ad_request_free(appsvr_ad_request_t request);

void appsvr_ad_request_real_free(appsvr_ad_request_t request);    

#ifdef __cplusplus
}
#endif

#endif
