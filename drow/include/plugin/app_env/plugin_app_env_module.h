#ifndef PLUGIN_APP_ENV_MODULE_H
#define PLUGIN_APP_ENV_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin_app_env_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_app_env_module_t
plugin_app_env_module_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_app_env_module_free(plugin_app_env_module_t module);

plugin_app_env_module_t plugin_app_env_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_app_env_module_t plugin_app_env_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_app_env_module_app(plugin_app_env_module_t module);
const char * plugin_app_env_module_name(plugin_app_env_module_t module);

uint8_t plugin_app_env_module_suspend(plugin_app_env_module_t module);
void plugin_app_env_module_set_suspend(plugin_app_env_module_t module, uint8_t is_suspend);

/*执行命令 */    
void plugin_app_env_post_request(
    plugin_app_env_module_t module,
    LPDRMETA meta, void const * data, size_t data_size);

int plugin_app_env_exec_request_sync(
    plugin_app_env_module_t module,
    dr_data_t * result, mem_buffer_t result_alloc,
    LPDRMETA meta, void const * data, size_t data_size);

int plugin_app_env_exec_request_asnyc(
    plugin_app_env_module_t module, uint32_t * id,
    void * receiver_ctx, plugin_app_env_request_receiver_fun_t receiver_fun, void (*ctx_free)(void *),
    LPDRMETA meta, void const * data, size_t data_size);

/*发送通知 */
uint32_t plugin_app_env_send_notification(plugin_app_env_module_t module, LPDRMETA meta, void const * data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif
