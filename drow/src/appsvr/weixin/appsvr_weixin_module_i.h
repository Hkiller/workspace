#ifndef APPSVR_STATISTICS_WEIXIN_MODULE_H
#define APPSVR_STATISTICS_WEIXIN_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "gd/net_trans/net_trans_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/account/appsvr_account_types.h"
#include "appsvr/payment/appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_weixin_module * appsvr_weixin_module_t;
typedef struct appsvr_weixin_backend * appsvr_weixin_backend_t;

struct appsvr_weixin_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;

    net_trans_group_t m_trans_group;
    
    appsvr_account_module_t m_account_module;
    appsvr_account_adapter_t m_account_adapter;

    appsvr_weixin_backend_t m_backend;

    char * m_appid;
    char * m_secret;
    char * m_scope;
    uint32_t m_login_session;
    
    struct mem_buffer m_dump_buffer;
};

int appsvr_weixin_backend_init(appsvr_weixin_module_t module);
void appsvr_weixin_backend_fini(appsvr_weixin_module_t module);
int appsvr_weixin_backend_start_login(appsvr_weixin_module_t module, uint8_t is_relogin);    
void appsvr_weixin_notify_login_result(appsvr_weixin_module_t module, const char * code, uint32_t session, int err, const char * errstr);
                                       
int appsvr_weixin_login_init(appsvr_weixin_module_t module);
void appsvr_weixin_login_fini(appsvr_weixin_module_t module);    

#ifdef __cplusplus
}
#endif

#endif
