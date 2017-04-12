#ifndef PLUGIN_APP_ENV_TYPES_H
#define PLUGIN_APP_ENV_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/pal/pal_types.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_app_env_module * plugin_app_env_module_t;
typedef struct plugin_app_env_executor * plugin_app_env_executor_t;
typedef struct plugin_app_env_monitor * plugin_app_env_monitor_t;    
typedef struct plugin_app_env_request * plugin_app_env_request_t;    
typedef struct plugin_app_env_executor_def * plugin_app_env_executor_def_t;

typedef void (* plugin_app_env_request_receiver_fun_t)(void * ctx, uint32_t id, int rv, dr_data_t result);

typedef enum plugin_app_env_executor_type {
    plugin_app_env_executor_oneway,
    plugin_app_env_executor_sync,
    plugin_app_env_executor_async,    
} plugin_app_env_executor_type_t;
    
#ifdef __cplusplus
}
#endif

#endif
