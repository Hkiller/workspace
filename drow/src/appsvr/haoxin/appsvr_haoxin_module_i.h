#ifndef APPSVR_STATISTICS_HAOXIN_MODULE_H
#define APPSVR_STATISTICS_HAOXIN_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "gd/app_attr/app_attr_types.h"
#include "gd/timer/timer_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/account/appsvr_account_types.h"
#include "appsvr/payment/appsvr_payment_types.h"
#include "appsvr/device/appsvr_device_types.h"
#include "protocol/appsvr/payment/appsvr_payment_pro.h"
#include "protocol/appsvr/sdk/appsvr_sdk_pro.h"
#include "protocol/appsvr/haoxin/appsvr_haoxin_pro.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct appsvr_haoxin_module * appsvr_haoxin_module_t;
    typedef struct appsvr_haoxin_backend * appsvr_haoxin_backend_t;

    struct appsvr_haoxin_module {
        gd_app_context_t m_app;
        mem_allocrator_t m_alloc;
        error_monitor_t m_em;
        app_attr_module_t m_app_attr_module;
        plugin_app_env_module_t m_app_env;
        uint8_t m_debug;

        /* */
        appsvr_haoxin_backend_t m_backend;

        /*属性 */
        APPSVR_HAOXIN_ATTR m_attr_data;
        LPDRMETA m_attr_meta;
        app_attr_provider_t m_attr_provider;

        /*状态维护 */
        plugin_app_env_monitor_t m_suspend_monitor;

        /*支付 */
        appsvr_payment_module_t m_payment_module;
        uint8_t m_payment_runing;
        appsvr_payment_adapter_t m_payment_adapter;
        gd_timer_id_t m_payment_cancel_timer;

        uint32_t m_runing_id;
        gd_timer_id_t m_runing_cancel_timer;

        /*页面 */
        plugin_app_env_monitor_t m_sdk_action_monitor;
        LPDRMETA m_meta_sdk_action;

        struct mem_buffer m_dump_buffer;
    };

    /* */
    int appsvr_haoxin_backend_init(appsvr_haoxin_module_t module);
    void appsvr_haoxin_backend_fini(appsvr_haoxin_module_t module);

    /*状态维护 */
    int appsvr_haoxin_suspend_monitor_init(appsvr_haoxin_module_t module);
    void appsvr_haoxin_suspend_monitor_fini(appsvr_haoxin_module_t module);
    void appsvr_haoxin_on_suspend(appsvr_haoxin_module_t module);
    void appsvr_haoxin_on_resume(appsvr_haoxin_module_t module);

    /*支付相关 */
    int appsvr_haoxin_payment_init(appsvr_haoxin_module_t module);
    void appsvr_haoxin_payment_fini(appsvr_haoxin_module_t module);
    int appsvr_haoxin_backend_pay_start(appsvr_haoxin_module_t module, APPSVR_PAYMENT_BUY const * req);
    void appsvr_haoxin_module_send_payment_result(appsvr_haoxin_module_t module, APPSVR_PAYMENT_RESULT const * payment_result);
    int appsvr_haoxin_module_start_payment_cancel_timer(appsvr_haoxin_module_t module);

    /*提供属性 */
    int appsvr_haoxin_attr_provider_init(appsvr_haoxin_module_t module);
    void appsvr_haoxin_attr_provider_fini(appsvr_haoxin_module_t module);
    int appsvr_haoxin_notify_support_more_game(appsvr_haoxin_module_t module, uint8_t is_support);
    int appsvr_haoxin_notify_support_exit_game(appsvr_haoxin_module_t module, uint8_t is_support);
    int appsvr_haoxin_notify_payscreen(appsvr_haoxin_module_t module, const char * payscreen);

#ifdef __cplusplus
}
#endif

#endif
