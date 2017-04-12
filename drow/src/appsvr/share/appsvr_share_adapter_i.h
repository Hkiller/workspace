#ifndef APPSVR_SHARE_ADAPTER_I_H
#define APPSVR_SHARE_ADAPTER_I_H
#include "appsvr/share/appsvr_share_adapter.h"
#include "appsvr_share_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_share_adapter {
    appsvr_share_module_t m_module;
    TAILQ_ENTRY(appsvr_share_adapter) m_next;
    appsvr_share_request_list_t m_requests;
    char m_name[32];
    void * m_ctx;
    appsvr_share_commit_fun_t m_commit_fun;
};
    
#ifdef __cplusplus
}
#endif

#endif
