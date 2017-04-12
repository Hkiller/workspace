#ifndef APPSVR_NOTIFY_DEVICE_MODULE_H
#define APPSVR_NOTIFY_DEVICE_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/notify/appsvr_notify_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_notify_device_module * appsvr_notify_device_module_t;
typedef struct appsvr_notify_device_backend * appsvr_notify_device_backend_t;

struct appsvr_notify_device_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    plugin_app_env_module_t m_app_env;
    appsvr_notify_device_backend_t m_backend;
    plugin_app_env_monitor_t m_suspend_monitor;
    
    char * m_tags;
    
    appsvr_notify_adapter_t m_notify_adapter;
    appsvr_notify_module_t m_notify_module;
};

int appsvr_notify_device_backend_init(appsvr_notify_device_module_t module);
void appsvr_notify_device_backend_fini(appsvr_notify_device_module_t module);

int appsvr_notify_device_adapter_init(appsvr_notify_device_module_t module);
void appsvr_notify_device_adapter_fini(appsvr_notify_device_module_t module);

int appsvr_notify_device_suspend_monitor_init(appsvr_notify_device_module_t module);
void appsvr_notify_device_suspend_monitor_fini(appsvr_notify_device_module_t module);
    
void appsvr_notify_device_on_suspend(appsvr_notify_device_module_t module);
void appsvr_notify_device_on_resume(appsvr_notify_device_module_t module);
    
int appsvr_notify_device_install_schedule(void * ctx, appsvr_notify_schedule_t schedule);
int appsvr_notify_device_update_schedule(void * ctx, appsvr_notify_schedule_t schedule);
void appsvr_notify_device_uninstall_schedule(void * ctx, appsvr_notify_schedule_t schedule);
    
#ifdef __cplusplus
}
#endif

#endif
