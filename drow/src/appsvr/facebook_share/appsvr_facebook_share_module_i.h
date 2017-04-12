#ifndef APPSVR_FACEBOOK_SHARE_MODULE_H
#define APPSVR_FACEBOOK_SHARE_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/share/appsvr_share_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_facebook_share_module * appsvr_facebook_share_module_t;
typedef struct appsvr_facebook_share_backend * appsvr_facebook_share_backend_t;

struct appsvr_facebook_share_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    plugin_app_env_monitor_t m_suspend_monitor;
    plugin_app_env_module_t m_app_env;

    appsvr_share_module_t m_share_module;
    appsvr_share_adapter_t m_share_adapter;

    appsvr_facebook_share_backend_t m_backend;
};

int appsvr_facebook_share_backend_init(appsvr_facebook_share_module_t module);
void appsvr_facebook_share_backend_fini(appsvr_facebook_share_module_t module);

int appsvr_facebook_share_commit_init(appsvr_facebook_share_module_t module);
void appsvr_facebook_share_commit_fini(appsvr_facebook_share_module_t module);
    
int appsvr_facebook_share_backend_commit(appsvr_facebook_share_module_t module, appsvr_share_request_t request);
    
#ifdef __cplusplus
}
#endif

#endif
