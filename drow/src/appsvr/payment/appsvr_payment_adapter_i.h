#ifndef APPSVR_PAYMENT_ADAPTER_I_H
#define APPSVR_PAYMENT_ADAPTER_I_H
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_payment_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum appsvr_payment_adapter_product_sync_state {
    appsvr_payment_adapter_product_sync_init,
    appsvr_payment_adapter_product_sync_runing,
    appsvr_payment_adapter_product_sync_done,
} appsvr_payment_adapter_product_sync_state_t;
    
struct appsvr_payment_adapter {
    appsvr_payment_module_t m_module;
    TAILQ_ENTRY(appsvr_payment_adapter) m_next;
    uint8_t m_service_type;
    char m_service_name[32];
    uint8_t m_support_restart;
    uint8_t m_support_sync;
    appsvr_payment_fini_fun_t m_fini;
    appsvr_payment_start_fun_t m_start_fun;
    appsvr_payment_query_products_fun_t m_query_products;
    appsvr_payment_product_list_t m_products;
    appsvr_payment_product_runing_list_t m_runings;    
    appsvr_payment_adapter_product_sync_state_t m_product_sync_state;
};
    
#ifdef __cplusplus
}
#endif

#endif
