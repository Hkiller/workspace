#ifndef APPSVR_PAYMENT_PRODUCT_I_H
#define APPSVR_PAYMENT_PRODUCT_I_H
#include "appsvr/payment/appsvr_payment_product.h"
#include "appsvr_payment_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_payment_product {
    appsvr_payment_module_t m_module;
    TAILQ_ENTRY(appsvr_payment_product) m_next_for_module;
    appsvr_payment_adapter_t m_adapter;
    TAILQ_ENTRY(appsvr_payment_product) m_next_for_adapter;
    char * m_product_id;
    char * m_price;
};
    
#ifdef __cplusplus
}
#endif

#endif
