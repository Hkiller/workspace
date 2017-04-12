#ifndef APPSVR_PAYMENT_PRODUCT_REQUEST_I_H
#define APPSVR_PAYMENT_PRODUCT_REQUEST_I_H
#include "appsvr_payment_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_payment_product_request {
    appsvr_payment_module_t m_module;
    TAILQ_ENTRY(appsvr_payment_product_request) m_next_for_module;
    void * m_ctx;
    appsvr_payment_product_resonse_fun_t m_response_fun;
    void * m_arg;
    void (*m_arg_free_fun)(void * ctx);
    appsvr_payment_product_runing_list_t m_runings;
};

appsvr_payment_product_request_t 
appsvr_payment_product_request_create(
    appsvr_payment_module_t module,
    void * ctx, appsvr_payment_product_resonse_fun_t response_fun, void * arg, void (*arg_free_fun)(void * ctx));

void appsvr_payment_product_request_free(appsvr_payment_product_request_t product_request);
    
#ifdef __cplusplus
}
#endif

#endif
