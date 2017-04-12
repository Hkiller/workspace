#ifndef APPSVR_IAPPPAY_MODULE_H
#define APPSVR_IAPPPAY_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "protocol/appsvr/payment/appsvr_payment_pro.h"
#include "appsvr/payment/appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_unicompay_module * appsvr_unicompay_module_t;
typedef struct appsvr_unicompay_payment_adapter * appsvr_unicompay_payment_adapter_t;
typedef struct appsvr_unicompay_backend * appsvr_unicompay_backend_t;

struct appsvr_unicompay_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    appsvr_payment_module_t m_payment_module;

    char * m_chanel;
    char * m_url;
    char * m_app_id;
    char * m_appv_key;
    char * m_platp_key;
    char * m_free_pay_product_id;
    
    uint8_t m_debug;
    appsvr_payment_adapter_t m_adapter;
    appsvr_unicompay_backend_t m_backend;
    struct mem_buffer m_dump_buffer;
};

int appsvr_unicompay_module_set_chanel(appsvr_unicompay_module_t adapter, const char * chanel);    
int appsvr_unicompay_module_set_url(appsvr_unicompay_module_t adapter, const char * url);
int appsvr_unicompay_module_set_free_pay_product_id(appsvr_unicompay_module_t adapter, const char * free_pay_product_id);

int appsvr_unicompay_module_set_app_id(appsvr_unicompay_module_t adapter, const char * app_id);
int appsvr_unicompay_module_set_appv_key(appsvr_unicompay_module_t adapter, const char * appv_key);
int appsvr_unicompay_module_set_platp_key(appsvr_unicompay_module_t adapter, const char * platp_key);

int appsvr_unicompay_payment_adapter_init(appsvr_unicompay_module_t adapter);
void appsvr_unicompay_payment_adapter_fini(appsvr_unicompay_module_t adapter);
    
int appsvr_unicompay_backend_init(appsvr_unicompay_module_t adapter);
void appsvr_unicompay_backend_fini(appsvr_unicompay_module_t adapter);

#ifdef __cplusplus
}
#endif

#endif
