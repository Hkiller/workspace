#ifndef PLUGIN_UI_POPUP_ACTION_I_H
#define PLUGIN_UI_POPUP_ACTION_I_H
#include "plugin/ui/plugin_ui_popup_action.h"
#include "plugin_ui_popup_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_action {
    plugin_ui_popup_t m_popup;
    TAILQ_ENTRY(plugin_ui_popup_action) m_next;
    char m_name[32];
    plugin_ui_popup_action_fun_t m_fun;
    void * m_ctx;
    char m_data[PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY];
};

void plugin_ui_popup_action_real_free(plugin_ui_popup_action_t action);

#ifdef __cplusplus
}
#endif

#endif
