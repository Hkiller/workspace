#ifndef APPSVR_STATISTICS_YOMOBADS_MODULE_H
#define APPSVR_STATISTICS_YOMOBADS_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/ad/appsvr_ad_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_yomobads_module * appsvr_yomobads_module_t;
typedef struct appsvr_yomobads_backend * appsvr_yomobads_backend_t;

struct appsvr_yomobads_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;
    plugin_app_env_monitor_t m_suspend_monitor;
    plugin_app_env_monitor_t m_pause_monitor;

    appsvr_ad_module_t m_ad_module;
    char * m_app_id;
    appsvr_yomobads_backend_t m_backend;
    
    uint32_t m_request_id;
    appsvr_ad_adapter_t m_ad_adapter;
};

/*ad adapter*/
int appsvr_yomobads_backend_open_start(appsvr_yomobads_module_t module, const char* sceneID);

int appsvr_yomobads_module_set_app_id(appsvr_yomobads_module_t module, const char * app_id);
    
int appsvr_yomobads_backend_init(appsvr_yomobads_module_t module);
void appsvr_yomobads_backend_fini(appsvr_yomobads_module_t module);

int appsvr_yomobads_ad_init(appsvr_yomobads_module_t module);
void appsvr_yomobads_ad_fini(appsvr_yomobads_module_t module);

void appsvr_yomobads_on_suspend(appsvr_yomobads_module_t module);
void appsvr_yomobads_on_resume(appsvr_yomobads_module_t module);
int appsvr_yomobads_suspend_monitor_init(appsvr_yomobads_module_t module);
void appsvr_yomobads_suspend_monitor_fini(appsvr_yomobads_module_t module);

int appsvr_yomobads_read_adaction_data(appsvr_yomobads_module_t module,cfg_t cfg);
#ifdef __cplusplus
}
#endif

#endif
