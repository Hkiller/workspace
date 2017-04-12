#ifndef DROW_PLUGIN_UI_ANIMATION_MOVE_OUT_H
#define DROW_PLUGIN_UI_ANIMATION_MOVE_OUT_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_MOVE_OUT;
    
plugin_ui_anim_control_move_out_t plugin_ui_anim_control_move_out_create(plugin_ui_control_t control);

int plugin_ui_anim_control_move_out_set_decorator(plugin_ui_anim_control_move_out_t move_out, const char * decorator);
void plugin_ui_anim_control_move_out_set_take_time(plugin_ui_anim_control_move_out_t move_out, float take_time);
void plugin_ui_anim_control_move_out_set_take_time_frame(plugin_ui_anim_control_move_out_t move_out, uint32_t take_time_frame);    
void plugin_ui_anim_control_move_out_set_target_pos(plugin_ui_anim_control_move_out_t move_out, plugin_ui_control_move_pos_t target);
    
#ifdef __cplusplus
}
#endif

#endif
