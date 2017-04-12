#ifndef DROW_PLUGIN_UI_ANIMATION_SCALE_OUT_H
#define DROW_PLUGIN_UI_ANIMATION_SCALE_OUT_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_SCALE_OUT;
    
plugin_ui_anim_control_scale_out_t plugin_ui_anim_control_scale_out_create(plugin_ui_control_t control);

int plugin_ui_anim_control_scale_out_set_decorator(plugin_ui_anim_control_scale_out_t scale_out, const char * decorator);
void plugin_ui_anim_control_scale_out_set_take_time(plugin_ui_anim_control_scale_out_t scale_out, float take_time);
void plugin_ui_anim_control_scale_out_set_target_scale(plugin_ui_anim_control_scale_out_t scale_out, ui_vector_2_t scale);
void plugin_ui_anim_control_scale_out_add_layer(plugin_ui_anim_control_scale_out_t scale_out, plugin_ui_control_frame_layer_t layer);
    
#ifdef __cplusplus
}
#endif

#endif

