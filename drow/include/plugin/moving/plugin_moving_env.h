#ifndef DROW_PLUGIN_MOVING_ENV_H
#define DROW_PLUGIN_MOVING_ENV_H
#include "plugin_moving_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_moving_env_t plugin_moving_env_create(plugin_moving_module_t module);
void plugin_moving_env_free(plugin_moving_env_t env);
ui_data_mgr_t plugin_moving_env_data_mgr(plugin_moving_env_t env);
    
#ifdef __cplusplus
}
#endif

#endif

