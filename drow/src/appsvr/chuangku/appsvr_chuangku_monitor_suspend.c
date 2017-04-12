#include <assert.h>
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "protocol/plugin/app_env/app_env_pro.h"
#include "appsvr_chuangku_module_i.h"

static int appsvr_chuangku_do_suspend(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_chuangku_module_t module = (appsvr_chuangku_module_t)ctx;
    APP_ENV_SUSPEND const * req = req_data; 

    if (req->is_suspend) {
        appsvr_chuangku_on_suspend(module);
    }
    else {
        if (module->m_payment_runing) {
            CPE_ERROR(module->m_em, "appsvr_chuangku_module_start_payment_cancel_timer: enter!");

            appsvr_chuangku_module_start_payment_cancel_timer(module);
        }
        appsvr_chuangku_on_resume(module);
    }

    return 0;
}

int appsvr_chuangku_suspend_monitor_init(appsvr_chuangku_module_t module) {
    module->m_suspend_monitor = 
        plugin_app_env_monitor_create(
        module->m_app_env,
        "app_env_suspend",
        module, appsvr_chuangku_do_suspend, NULL);
    if (module->m_suspend_monitor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_plugin_init: create suspend fail!");
        return -1;
    }

    return 0;
}

void appsvr_chuangku_suspend_monitor_fini(appsvr_chuangku_module_t module) {
    assert(module->m_suspend_monitor);
    plugin_app_env_monitor_free(module->m_suspend_monitor);
    module->m_suspend_monitor = NULL;
}