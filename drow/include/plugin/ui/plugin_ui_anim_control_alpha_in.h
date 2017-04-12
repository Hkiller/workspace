#ifndef DROW_PLUGIN_UI_ANIMATION_ALPHA_IN_H
#define DROW_PLUGIN_UI_ANIMATION_ALPHA_IN_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_ALPHA_IN;
    
plugin_ui_anim_control_alpha_in_t plugin_ui_anim_control_alpha_in_create(plugin_ui_control_t control);

int plugin_ui_anim_control_alpha_in_set_decorator(plugin_ui_anim_control_alpha_in_t alpha_in, const char * decorator);
void plugin_ui_anim_control_alpha_in_set_take_time(plugin_ui_anim_control_alpha_in_t alpha_in, float take_time);
void plugin_ui_anim_control_alpha_in_set_take_time_frame(plugin_ui_anim_control_alpha_in_t alpha_in, uint32_t take_time_frame);
void plugin_ui_anim_control_alpha_in_set_origin(plugin_ui_anim_control_alpha_in_t alpha_in, float origin);
void plugin_ui_anim_control_alpha_in_add_layer(plugin_ui_anim_control_alpha_in_t alpha_in, plugin_ui_control_frame_layer_t layer);
    
#ifdef __cplusplus
}
#endif

#endif

