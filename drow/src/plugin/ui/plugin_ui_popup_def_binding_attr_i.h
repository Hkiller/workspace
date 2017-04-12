#ifndef PLUGIN_UI_POPUP_DEF_BINDING_ATTR_I_H
#define PLUGIN_UI_POPUP_DEF_BINDING_ATTR_I_H
#include "plugin/ui/plugin_ui_popup_def_binding_attr.h"
#include "plugin_ui_popup_def_binding_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_def_binding_attr {
    plugin_ui_popup_def_binding_t m_binding;
    TAILQ_ENTRY(plugin_ui_popup_def_binding_attr) m_next;
    char * m_attr_name;
    char * m_attr_value;
};

#ifdef __cplusplus
}
#endif

#endif
