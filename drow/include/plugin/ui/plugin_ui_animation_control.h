#ifndef DROW_PLUGIN_UI_ANIMATION_CONTROL_H
#define DROW_PLUGIN_UI_ANIMATION_CONTROL_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_animation_control_t
plugin_ui_animation_control_create(
    plugin_ui_animation_t animation, plugin_ui_control_t control, uint8_t is_tie);
void plugin_ui_animation_control_free(plugin_ui_animation_control_t animation);

uint8_t plugin_ui_animation_control_is_tie(plugin_ui_animation_control_t animation_control);

plugin_ui_animation_control_t
plugin_ui_animation_control_find(
    plugin_ui_animation_t animation, plugin_ui_control_t control);

void * plugin_ui_animation_control_data(plugin_ui_animation_control_t animation_control);
plugin_ui_animation_control_t plugin_ui_animation_control_from_data(void * data);
    
#ifdef __cplusplus
}
#endif

#endif

