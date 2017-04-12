#ifndef APPSVR_PAYMENT_PRODUCT_RUNING_I_H
#define APPSVR_PAYMENT_PRODUCT_RUNING_I_H
#include "appsvr_payment_product_request_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_payment_product_runing {
    appsvr_payment_product_request_t m_request;
    TAILQ_ENTRY(appsvr_payment_product_runing) m_next_for_request;
    appsvr_payment_adapter_t m_adapter;
    TAILQ_ENTRY(appsvr_payment_product_runing) m_next_for_adapter;
};

appsvr_payment_product_runing_t 
appsvr_payment_product_runing_create(appsvr_payment_product_request_t request, appsvr_payment_adapter_t adapter);
void appsvr_payment_product_runing_free(appsvr_payment_product_runing_t product_runing);
    
#ifdef __cplusplus
}
#endif

#endif
