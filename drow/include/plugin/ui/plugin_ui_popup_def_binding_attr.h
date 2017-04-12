#ifndef DROW_PLUGIN_UI_POPUP_DEF_BINDING_ATTR_H
#define DROW_PLUGIN_UI_POPUP_DEF_BINDING_ATTR_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_def_binding_attr_it {
    plugin_ui_popup_def_binding_attr_t (*next)(struct plugin_ui_popup_def_binding_attr_it * it);
    char m_data[64];
};

plugin_ui_popup_def_binding_attr_t
plugin_ui_popup_def_binding_attr_create(plugin_ui_popup_def_binding_t binding, const char * attr_name, const char * attr_value);
void plugin_ui_popup_def_binding_attr_free(plugin_ui_popup_def_binding_attr_t binding_attr);

#define plugin_ui_popup_def_binding_attr_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

