#ifndef DROW_PLUGIN_UI_ANIMATION_FRAME_SCALE_H
#define DROW_PLUGIN_UI_ANIMATION_FRAME_SCALE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_FRAME_SCALE;
    
plugin_ui_anim_control_frame_scale_t plugin_ui_anim_control_frame_scale_create(plugin_ui_control_t control);

int plugin_ui_anim_control_frame_scale_set_decorator(plugin_ui_anim_control_frame_scale_t frame_scale, const char * decorator);
void plugin_ui_anim_control_frame_scale_set_take_time(plugin_ui_anim_control_frame_scale_t frame_scale, float take_time);
void plugin_ui_anim_control_frame_scale_set_target_scale(plugin_ui_anim_control_frame_scale_t frame_scale, ui_vector_2_t size);
    
#ifdef __cplusplus
}
#endif

#endif

