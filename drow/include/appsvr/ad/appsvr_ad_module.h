#ifndef APPSVR_AD_MODULE_H
#define APPSVR_AD_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin/ui/plugin_ui_types.h"
#include "appsvr_ad_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_ad_module_t
appsvr_ad_module_create(
    gd_app_context_t app, uint8_t debug, plugin_ui_module_t ui_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void appsvr_ad_module_free(appsvr_ad_module_t module);

appsvr_ad_module_t appsvr_ad_module_find(gd_app_context_t app, cpe_hash_string_t name);
appsvr_ad_module_t appsvr_ad_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t appsvr_ad_module_app(appsvr_ad_module_t module);
const char * appsvr_ad_module_name(appsvr_ad_module_t module);

int appsvr_ad_module_start(
    appsvr_ad_module_t module, const char * action, uint32_t * r_id,
    void * ctx, appsvr_ad_resonse_fun_t response_fun, void * arg, void (*arg_free_fun)(void * ctx));

int appsvr_ad_module_remove_by_id(appsvr_ad_module_t module, uint32_t request_id);

uint32_t appsvr_ad_module_remove_by_ctx(appsvr_ad_module_t module, void * ctx);
    
#ifdef __cplusplus
}
#endif

#endif
