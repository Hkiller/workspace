#ifndef APPSVR_NOTIFY_ADAPTER_I_H
#define APPSVR_NOTIFY_ADAPTER_I_H
#include "appsvr/notify/appsvr_notify_adapter.h"
#include "appsvr_notify_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_notify_adapter {
    appsvr_notify_module_t m_module;
    TAILQ_ENTRY(appsvr_notify_adapter) m_next;
    appsvr_notify_tag_adapter_list_t m_tags;
    appsvr_notify_activate_list_t m_activates;
    char m_name[32];
    void * m_ctx;
    appsvr_notify_install_fun_t m_install_fun;
    appsvr_notify_update_fun_t m_update_fun;
    appsvr_notify_uninstall_fun_t m_uninstall_fun;
};
    
#ifdef __cplusplus
}
#endif

#endif
