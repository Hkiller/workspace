#ifndef APPSVR_ACCOUNT_MODULE_I_H
#define APPSVR_ACCOUNT_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/account/appsvr_account_module.h"
#include "protocol/appsvr/account/appsvr_account_pro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(appsvr_account_adapter_list, appsvr_account_adapter) appsvr_account_adapter_list_t;
                                                                        
struct appsvr_account_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    LPDRMETA m_meta_req_login;
    LPDRMETA m_meta_res_login;
    LPDRMETA m_meta_req_relogin;
    LPDRMETA m_meta_req_query_services;
    LPDRMETA m_meta_res_query_services;
    uint8_t m_debug;

    uint8_t m_adapter_count;
    appsvr_account_adapter_list_t m_adapters;

    appsvr_account_adapter_t m_runing_adapter;
    uint32_t m_runing_id;
    
    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
