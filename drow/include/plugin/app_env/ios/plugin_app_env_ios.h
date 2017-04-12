#ifndef PLUGIN_APP_ENV_IOS_H
#define PLUGIN_APP_ENV_IOS_H
#include "../plugin_app_env_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void plugin_app_env_ios_set_window(plugin_app_env_module_t app_env, void * window);    
void * plugin_app_env_ios_window(plugin_app_env_module_t app_env);

void plugin_app_env_ios_set_application(plugin_app_env_module_t app_env, void * application);    
void * plugin_app_env_ios_application(plugin_app_env_module_t app_env);

typedef uint8_t (*plugin_app_env_ios_open_url_fun_t)(void * ctx, void * application, void * url, void * sourceApplication, void * annotation);

int plugin_app_env_ios_register_delegate(
    plugin_app_env_module_t app_env,
    void * ctx,
    plugin_app_env_ios_open_url_fun_t open_url);

void plugin_app_env_ios_unregister_delegate(
    plugin_app_env_module_t app_env,
    void * ctx);

uint8_t plugin_app_env_ios_dispatch_open_url(
    plugin_app_env_module_t app_env,
    void * application, void * url, void * sourceApplication, void * annotation);
    
#ifdef __cplusplus
}
#endif

#endif

