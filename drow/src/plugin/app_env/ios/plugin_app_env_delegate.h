#ifndef PLUGIN_APP_ENV_DELEGATE_I_H
#define PLUGIN_APP_ENV_DELEGATE_I_H
#include "plugin_app_env_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_app_env_delegator {
    plugin_app_env_backend_t m_backend;
    TAILQ_ENTRY(plugin_app_env_delegator) m_next;
    void * m_ctx;
    plugin_app_env_ios_open_url_fun_t m_open_url;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
