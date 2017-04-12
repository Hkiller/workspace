#ifndef APPSVR_ACCOUNT_ADAPTER_I_H
#define APPSVR_ACCOUNT_ADAPTER_I_H
#include "appsvr/account/appsvr_account_adapter.h"
#include "appsvr_account_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_account_adapter {
    appsvr_account_module_t m_module;
    TAILQ_ENTRY(appsvr_account_adapter) m_next;
    uint8_t m_service_type;
    char m_service_name[32];
    appsvr_account_fini_fun_t m_fini;
    appsvr_account_login_start_fun_t m_login_start_fun;
    appsvr_account_relogin_start_fun_t m_relogin_start_fun;
};
    
#ifdef __cplusplus
}
#endif

#endif
