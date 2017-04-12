#ifndef APPSVR_PAYMENT_MODULE_I_H
#define APPSVR_PAYMENT_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/payment/appsvr_payment_module.h"
#include "protocol/appsvr/payment/appsvr_payment_pro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_payment_product_request * appsvr_payment_product_request_t;
typedef struct appsvr_payment_product_runing * appsvr_payment_product_runing_t;

typedef TAILQ_HEAD(appsvr_payment_adapter_list, appsvr_payment_adapter) appsvr_payment_adapter_list_t;
typedef TAILQ_HEAD(appsvr_payment_product_list, appsvr_payment_product) appsvr_payment_product_list_t;
typedef TAILQ_HEAD(appsvr_payment_product_runing_list, appsvr_payment_product_runing) appsvr_payment_product_runing_list_t;
typedef TAILQ_HEAD(appsvr_payment_product_request_list, appsvr_payment_product_request) appsvr_payment_product_request_list_t;

struct appsvr_payment_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    LPDRMETA m_meta_req_pay;
    LPDRMETA m_meta_res_pay;
    LPDRMETA m_meta_req_query_services;
    LPDRMETA m_meta_res_query_services;
    uint8_t m_debug;

    uint8_t m_adapter_count;
    appsvr_payment_adapter_list_t m_adapters;

    appsvr_payment_adapter_t m_runing_adapter;
    uint32_t m_runing_id;

    appsvr_payment_product_list_t m_products;
    appsvr_payment_product_request_list_t m_product_requests;
    
    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
