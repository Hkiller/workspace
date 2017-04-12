#ifndef DROW_PLUGIN_UI_ANIMATION_SCROLL_H
#define DROW_PLUGIN_UI_ANIMATION_SCROLL_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_CONTROL_SCROLL;
    
plugin_ui_anim_control_scroll_t plugin_ui_anim_control_scroll_create(plugin_ui_control_t control);

uint8_t plugin_ui_anim_control_scroll_guard_done(plugin_ui_anim_control_scroll_t scroll);
void plugin_ui_anim_control_scroll_set_guard_done(plugin_ui_anim_control_scroll_t scroll, uint8_t guard_done);
    
int plugin_ui_anim_control_scroll_set_decorator(plugin_ui_anim_control_scroll_t scroll, const char * decorator);
void plugin_ui_anim_control_scroll_set_take_time(plugin_ui_anim_control_scroll_t scroll, float take_time);
void plugin_ui_anim_control_scroll_set_target_x(plugin_ui_anim_control_scroll_t scroll, float x);
uint8_t plugin_ui_anim_control_scroll_process_x(plugin_ui_anim_control_scroll_t scroll);
void plugin_ui_anim_control_scroll_set_target_y(plugin_ui_anim_control_scroll_t scroll, float y);
uint8_t plugin_ui_anim_control_scroll_process_y(plugin_ui_anim_control_scroll_t scroll);
    
#ifdef __cplusplus
}
#endif

#endif
