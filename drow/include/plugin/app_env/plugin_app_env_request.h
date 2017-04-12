#ifndef PLUGIN_APP_ENV_REQUEST_H
#define PLUGIN_APP_ENV_REQUEST_H
#include "plugin_app_env_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_app_env_cancel_request_by_id(plugin_app_env_module_t module, uint32_t id); /*回调 */
uint32_t plugin_app_env_cancel_requests_by_req(plugin_app_env_module_t module, const char * req_name); /*回调 */

/*清理 */
uint32_t plugin_app_env_clear_requests_by_ctx(plugin_app_env_module_t module, void * receiver_ctx); /*不回调 */

int plugin_app_env_notify_request_result(
    plugin_app_env_module_t module, uint32_t id,
    int rv, LPDRMETA meta, void const * data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif
