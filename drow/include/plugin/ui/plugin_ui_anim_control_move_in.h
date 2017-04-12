#ifndef DROW_PLUGIN_UI_ANIMATION_MOVE_IN_H
#define DROW_PLUGIN_UI_ANIMATION_MOVE_IN_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_MOVE_IN;
    
plugin_ui_anim_control_move_in_t plugin_ui_anim_control_move_in_create(plugin_ui_control_t control);

int plugin_ui_anim_control_move_in_set_decorator(plugin_ui_anim_control_move_in_t move_in, const char * decorator);
void plugin_ui_anim_control_move_in_set_take_time(plugin_ui_anim_control_move_in_t move_in, float take_time);
void plugin_ui_anim_control_move_in_set_take_time_frame(plugin_ui_anim_control_move_in_t move_in, uint32_t take_time_frame);
void plugin_ui_anim_control_move_in_set_origin_pos(plugin_ui_anim_control_move_in_t move_in, plugin_ui_control_move_pos_t pos);
void plugin_ui_anim_control_move_in_set_begin_at(plugin_ui_anim_control_move_in_t move_in, float begin_at);
    
#ifdef __cplusplus
}
#endif

#endif

