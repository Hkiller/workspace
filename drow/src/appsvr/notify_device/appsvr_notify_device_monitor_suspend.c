#include <assert.h>
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "protocol/plugin/app_env/app_env_pro.h"
#include "appsvr_notify_device_module_i.h"

static int appsvr_notify_device_do_suspend(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_notify_device_module_t module = (appsvr_notify_device_module_t)ctx;
    APP_ENV_SUSPEND const * req = req_data; 

    if (req->is_suspend) {
        appsvr_notify_device_on_suspend(module);
    }
    else {
        appsvr_notify_device_on_resume(module);
    }

    return 0;
}

int appsvr_notify_device_suspend_monitor_init(appsvr_notify_device_module_t module) {
    module->m_suspend_monitor = 
        plugin_app_env_monitor_create(
            module->m_app_env,
            "app_env_suspend",
            module, appsvr_notify_device_do_suspend, NULL);
    if (module->m_suspend_monitor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_notify_device_suspend_monitor_init: create suspend fail!");
        return -1;
    }

    return 0;
}

void appsvr_notify_device_suspend_monitor_fini(appsvr_notify_device_module_t module) {
    assert(module->m_suspend_monitor);
    plugin_app_env_monitor_free(module->m_suspend_monitor);
    module->m_suspend_monitor = NULL;
}

