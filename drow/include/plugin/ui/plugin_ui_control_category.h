#ifndef DROW_PLUGIN_UI_CONTROL_CATEGORY_H
#define DROW_PLUGIN_UI_CONTROL_CATEGORY_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_control_category_t plugin_ui_control_category_create(plugin_ui_env_t env, const char * prefix);
void plugin_ui_control_category_free(plugin_ui_control_category_t control_category);

plugin_ui_control_category_t plugin_ui_control_category_find(plugin_ui_env_t env, const char * prefix);

int plugin_ui_control_category_set_click_audio(plugin_ui_control_category_t category, const char * res);

#ifdef __cplusplus
}
#endif

#endif

