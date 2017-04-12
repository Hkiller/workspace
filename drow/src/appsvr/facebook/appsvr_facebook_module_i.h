#ifndef APPSVR_STATISTICS_facebook_MODULE_H
#define APPSVR_STATISTICS_facebook_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/account/appsvr_account_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_facebook_permission * appsvr_facebook_permission_t;    
typedef struct appsvr_facebook_module * appsvr_facebook_module_t;
typedef struct appsvr_facebook_backend * appsvr_facebook_backend_t;

typedef TAILQ_HEAD(appsvr_facebook_permission_list, appsvr_facebook_permission) appsvr_facebook_permission_list_t;
    
struct appsvr_facebook_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;
    plugin_app_env_monitor_t m_suspend_monitor;

    appsvr_account_module_t m_account_module;
    appsvr_account_adapter_t m_account_adapter;
    appsvr_facebook_permission_list_t m_permissions;
    
	char * m_token;

    appsvr_facebook_backend_t m_backend;
    
    struct mem_buffer m_dump_buffer;
};

int appsvr_facebook_set_token(appsvr_facebook_module_t module, const char * token);

int appsvr_facebook_backend_init(appsvr_facebook_module_t module);
void appsvr_facebook_backend_fini(appsvr_facebook_module_t module);

int appsvr_facebook_backend_login_start(appsvr_facebook_module_t module, uint8_t is_relogin);
int appsvr_facebook_login_init(appsvr_facebook_module_t module);
void appsvr_facebook_login_fini(appsvr_facebook_module_t module);

void appsvr_facebook_on_suspend(appsvr_facebook_module_t module);
void appsvr_facebook_on_resume(appsvr_facebook_module_t module);
int appsvr_facebook_suspend_monitor_init(appsvr_facebook_module_t module);
void appsvr_facebook_suspend_monitor_fini(appsvr_facebook_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
