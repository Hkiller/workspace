#ifndef APPSVR_STATISTICS_APPLE_PURCHASE_MODULE_H
#define APPSVR_STATISTICS_APPLE_PURCHASE_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "gd/timer/timer_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/payment/appsvr_payment_types.h"
#include "protocol/appsvr/payment/appsvr_payment_pro.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct appsvr_apple_purchase_module * appsvr_apple_purchase_module_t;
typedef struct appsvr_apple_purchase_backend * appsvr_apple_purchase_backend_t;

struct appsvr_apple_purchase_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;

    appsvr_payment_module_t m_payment_module;
    appsvr_payment_adapter_t m_payment_adapter;
    gd_timer_id_t m_payment_success_timer;

    appsvr_apple_purchase_backend_t m_backend;

    struct mem_buffer m_dump_buffer;
};

/*payment adapter*/
int appsvr_apple_purchase_backend_pay_start(appsvr_apple_purchase_module_t module, APPSVR_PAYMENT_BUY const * req);
int appsvr_apple_purchase_backend_do_sync_products(appsvr_apple_purchase_module_t module);
int appsvr_apple_purchase_backend_init(appsvr_apple_purchase_module_t module);
void appsvr_apple_purchase_backend_fini(appsvr_apple_purchase_module_t module);

int appsvr_apple_purchase_payment_init(appsvr_apple_purchase_module_t module);
void appsvr_apple_purchase_payment_fini(appsvr_apple_purchase_module_t module);
int appsvr_apple_purchase_module_start_payment_success_timer(appsvr_apple_purchase_module_t module);

#ifdef __cplusplus
}
#endif

#endif
