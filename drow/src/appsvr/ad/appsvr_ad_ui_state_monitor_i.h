#ifndef APPSVR_AD_UI_STATE_MONITOR_I_H
#define APPSVR_AD_UI_STATE_MONITOR_I_H
#include "appsvr_ad_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_ad_ui_state_monitor {
    appsvr_ad_module_t m_module;
    TAILQ_ENTRY(appsvr_ad_ui_state_monitor) m_next;
    char m_state_name[32];
    char m_action_name[32];
};

appsvr_ad_ui_state_monitor_t appsvr_ad_ui_state_monitor_create(appsvr_ad_module_t module, const char * state_name, const char * action_name);
appsvr_ad_ui_state_monitor_t appsvr_ad_ui_state_monitor_find_by_state_name(appsvr_ad_module_t module, const char * state_name);
void appsvr_ad_ui_state_monitor_free(appsvr_ad_ui_state_monitor_t ui_state_monitor);

#ifdef __cplusplus
}
#endif

#endif
