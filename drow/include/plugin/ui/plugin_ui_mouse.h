#ifndef DROW_PLUGIN_UI_ENV_MOUSE_H
#define DROW_PLUGIN_UI_ENV_MOUSE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void plugin_ui_env_mouse_active(plugin_ui_env_t env);
void plugin_ui_env_mouse_deactive(plugin_ui_env_t env);
uint8_t plugin_ui_env_mouse_is_active(plugin_ui_env_t env);

plugin_ui_mouse_t plugin_ui_env_mouse(plugin_ui_env_t env);
plugin_ui_mouse_t plugin_ui_env_mouse_check_create(plugin_ui_env_t env);

ui_vector_2_t plugin_ui_env_mouse_pos(plugin_ui_mouse_t mouse);
void plugin_ui_env_mouse_set_pos(plugin_ui_mouse_t env, ui_vector_2_t pos);
uint8_t plugin_ui_mouse_l_down(plugin_ui_mouse_t mouse);
void plugin_ui_mouse_set_l_down(plugin_ui_mouse_t mouse, uint8_t is_down);
uint8_t plugin_ui_mouse_r_down(plugin_ui_mouse_t mouse);
void plugin_ui_mouse_set_r_down(plugin_ui_mouse_t mouse, uint8_t is_down);

#ifdef __cplusplus
}
#endif

#endif

