#ifndef PLUGIN_UI_POPUP_DEF_BINDING_I_H
#define PLUGIN_UI_POPUP_DEF_BINDING_I_H
#include "plugin/ui/plugin_ui_popup_def_binding.h"
#include "plugin_ui_popup_def_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_def_binding {
    plugin_ui_popup_def_t m_popup_def;
    TAILQ_ENTRY(plugin_ui_popup_def_binding) m_next;
    const char * m_control;
    uint8_t m_on_click_close;
    char * m_on_click_action;
    plugin_ui_popup_def_binding_attr_list_t m_binding_attrs;
};

int plugin_ui_popup_def_binding_apply(plugin_ui_popup_def_binding_t binding, plugin_ui_popup_t popup);
    
#ifdef __cplusplus
}
#endif

#endif
