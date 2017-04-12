#ifndef DROW_PLUGIN_UI_ANIMATION_FRAME_MOVE_H
#define DROW_PLUGIN_UI_ANIMATION_FRAME_MOVE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_FRAME_MOVE;
    
plugin_ui_anim_control_frame_move_t plugin_ui_anim_control_frame_move_create(plugin_ui_control_t control);

int plugin_ui_anim_control_frame_move_set_decorator(plugin_ui_anim_control_frame_move_t frame_move, const char * decorator);
void plugin_ui_anim_control_frame_move_set_take_time(plugin_ui_anim_control_frame_move_t frame_move, float take_time);
void plugin_ui_anim_control_frame_move_set_target_pos(plugin_ui_anim_control_frame_move_t frame_move, ui_vector_2_t pos);
    
#ifdef __cplusplus
}
#endif

#endif

