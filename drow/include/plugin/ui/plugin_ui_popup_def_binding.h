#ifndef DROW_PLUGIN_UI_POPUP_DEF_BINDING_H
#define DROW_PLUGIN_UI_POPUP_DEF_BINDING_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_def_binding_it {
    plugin_ui_popup_def_binding_t (*next)(struct plugin_ui_popup_def_binding_it * it);
    char m_data[64];
};

plugin_ui_popup_def_binding_t plugin_ui_popup_def_binding_create(plugin_ui_popup_def_t popup_def, const char * control);
void plugin_ui_popup_def_binding_free(plugin_ui_popup_def_binding_t binding);

const char * plugin_ui_popup_def_binding_on_click_action(plugin_ui_popup_def_binding_t binding);
int plugin_ui_popup_def_binding_set_on_click_action(plugin_ui_popup_def_binding_t binding, const char * res);

uint8_t plugin_ui_popup_def_binding_on_click_close(plugin_ui_popup_def_binding_t binding);
void plugin_ui_popup_def_binding_set_on_click_close(plugin_ui_popup_def_binding_t binding, uint8_t on_click_close);
    
#define plugin_ui_popup_def_binding_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

