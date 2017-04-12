#ifndef DROW_PLUGIN_UI_ANIMATION_CONTROL_MOVE_H
#define DROW_PLUGIN_UI_ANIMATION_CONTROL_MOVE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_MOVE;
    
plugin_ui_anim_control_move_t
plugin_ui_anim_control_move_create(plugin_ui_control_t control, char * arg_buf_will_change);

int plugin_ui_anim_control_move_set_decorator(plugin_ui_anim_control_move_t move, const char * decorator);
void plugin_ui_anim_control_move_set_take_time(plugin_ui_anim_control_move_t move, float take_time);
void plugin_ui_anim_control_move_set_take_time_frame(plugin_ui_anim_control_move_t move, uint32_t take_time_frame);
void plugin_ui_anim_control_move_set_algorithm(plugin_ui_anim_control_move_t move, plugin_ui_move_algorithm_t algorithm);
void plugin_ui_anim_control_move_set_algorithm_by_def(plugin_ui_anim_control_move_t move, const char * def);
int plugin_ui_anim_control_move_set_origin(plugin_ui_anim_control_move_t move, const char * origin);
int plugin_ui_anim_control_move_set_target(plugin_ui_anim_control_move_t move, const char * origin);
    
#ifdef __cplusplus
}
#endif

#endif

