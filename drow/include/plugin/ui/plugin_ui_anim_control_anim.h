#ifndef DROW_PLUGIN_UI_ANIM_CONTROL_ANIM_H
#define DROW_PLUGIN_UI_ANIM_CONTROL_ANIM_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_ANIM;

plugin_ui_anim_control_anim_t
plugin_ui_anim_control_anim_create(plugin_ui_control_t control);
plugin_ui_anim_control_anim_t
plugin_ui_anim_control_anim_create_with_setup(plugin_ui_control_t control, char * arg_buf_will_change);
    
ui_data_control_anim_t plugin_ui_anim_control_anim_data(plugin_ui_anim_control_anim_t anim);
int plugin_ui_anim_control_anim_set_data(plugin_ui_anim_control_anim_t anim, ui_data_control_anim_t data);

uint32_t plugin_ui_anim_control_anim_loop_count(plugin_ui_anim_control_anim_t anim);    
void plugin_ui_anim_control_anim_set_loop_count(plugin_ui_anim_control_anim_t anim, uint32_t loop_count);
    
#ifdef __cplusplus
}
#endif

#endif

