#ifndef PLUGIN_APP_ENV_BACKEND_I_H
#define PLUGIN_APP_ENV_BACKEND_I_H
#include "plugin/app_env/ios/plugin_app_env_ios.h"
#include "../plugin_app_env_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_app_env_delegator * plugin_app_env_delegator_t;
typedef TAILQ_HEAD(plugin_app_env_delegator_list,  plugin_app_env_delegator) plugin_app_env_delegator_list_t;
    
struct plugin_app_env_backend {
    plugin_app_env_module_t m_module;
    void * m_window;
    void * m_application;
    plugin_app_env_delegator_list_t m_delegators;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
