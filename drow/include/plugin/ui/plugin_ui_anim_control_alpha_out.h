#ifndef DROW_PLUGIN_UI_ANIMATION_ALPHA_OUT_H
#define DROW_PLUGIN_UI_ANIMATION_ALPHA_OUT_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT;
    
plugin_ui_anim_control_alpha_out_t plugin_ui_anim_control_alpha_out_create(plugin_ui_control_t control);

int plugin_ui_anim_control_alpha_out_set_decorator(plugin_ui_anim_control_alpha_out_t alpha_out, const char * decorator);
void plugin_ui_anim_control_alpha_out_set_take_time(plugin_ui_anim_control_alpha_out_t alpha_out, float take_time);
void plugin_ui_anim_control_alpha_out_set_take_time_frame(plugin_ui_anim_control_alpha_out_t alpha_out, uint32_t take_time_frame);
void plugin_ui_anim_control_alpha_out_set_target(plugin_ui_anim_control_alpha_out_t alpha_out, float origin);    
    
#ifdef __cplusplus
}
#endif

#endif

