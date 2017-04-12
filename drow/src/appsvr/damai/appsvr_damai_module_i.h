#ifndef APPSVR_STATISTICS_DAMAI_MODULE_H
#define APPSVR_STATISTICS_DAMAI_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/account/appsvr_account_types.h"
#include "appsvr/payment/appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_damai_module * appsvr_damai_module_t;
typedef struct appsvr_damai_backend * appsvr_damai_backend_t;

struct appsvr_damai_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;

    appsvr_account_module_t m_account_module;
    appsvr_payment_module_t m_payment_module;

    LPDRMETA m_userinfo_source_meta;
    char * m_userinfo_service;
    char * m_userinfo_role;
    char * m_userinfo_grade;
    
    plugin_app_env_monitor_t m_suspend_monitor;
    plugin_app_env_executor_t m_set_userinfo_executor;
    appsvr_damai_backend_t m_backend;
    xcomputer_t m_computer;
    
    struct mem_buffer m_dump_buffer;
};

int appsvr_damai_backend_init(appsvr_damai_module_t module);
void appsvr_damai_backend_fini(appsvr_damai_module_t module);

int appsvr_damai_set_userinfo_init(appsvr_damai_module_t module);
void appsvr_damai_set_userinfo_fini(appsvr_damai_module_t module);

int appsvr_damai_suspend_monitor_init(appsvr_damai_module_t module);
void appsvr_damai_suspend_monitor_fini(appsvr_damai_module_t module);
    
void appsvr_damai_on_suspend(appsvr_damai_module_t module);
void appsvr_damai_on_resume(appsvr_damai_module_t module);

void appsvr_damai_backend_set_userinfo(appsvr_damai_module_t module, const char * userinfo);
    
#ifdef __cplusplus
}
#endif

#endif
