#ifndef DROW_PLUGIN_UI_POPUP_ACTION_H
#define DROW_PLUGIN_UI_POPUP_ACTION_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_popup_action_t
plugin_ui_popup_action_create(
    plugin_ui_popup_t popup, const char * action_naem, plugin_ui_popup_action_fun_t fun, void * ctx);
void plugin_ui_popup_action_free(plugin_ui_popup_action_t action);
    
uint8_t plugin_ui_popup_action_data_capacity(plugin_ui_popup_action_t action);
void * plugin_ui_popup_action_data(plugin_ui_popup_action_t action);

#ifdef __cplusplus
}
#endif

#endif

