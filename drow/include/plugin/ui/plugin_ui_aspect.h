#ifndef DROW_PLUGIN_UI_CONTROL_ASPECT_H
#define DROW_PLUGIN_UI_CONTROL_ASPECT_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_aspect_t
plugin_ui_aspect_create(plugin_ui_env_t env, const char * name);
void plugin_ui_aspect_free(plugin_ui_aspect_t aspect);

void plugin_ui_aspect_clear(plugin_ui_aspect_t aspect);

int plugin_ui_aspect_env_action_add(plugin_ui_aspect_t aspect, plugin_ui_env_action_t action, uint8_t is_tie);
void plugin_ui_aspect_env_action_remove(plugin_ui_aspect_t aspect, plugin_ui_env_action_t action);    
uint8_t plugin_ui_aspect_env_action_is_in(plugin_ui_aspect_t aspect, plugin_ui_env_action_t action);        
void plugin_ui_aspect_env_action_clear(plugin_ui_aspect_t aspect);
void plugin_ui_aspect_env_actions(plugin_ui_env_action_it_t env_actions, plugin_ui_aspect_t aspect);
    
int plugin_ui_aspect_control_add(plugin_ui_aspect_t aspect, plugin_ui_control_t control, uint8_t is_tie);
void plugin_ui_aspect_control_remove(plugin_ui_aspect_t aspect, plugin_ui_control_t control);
uint8_t plugin_ui_aspect_control_is_in(plugin_ui_aspect_t aspect, plugin_ui_control_t control);
uint8_t plugin_ui_aspect_control_has_frame_in(plugin_ui_aspect_t aspect, plugin_ui_control_t control);
uint8_t plugin_ui_aspect_control_has_action_in(plugin_ui_aspect_t aspect, plugin_ui_control_t control);    
void plugin_ui_aspect_control_clear(plugin_ui_aspect_t aspect);
void plugin_ui_aspect_controls(plugin_ui_control_it_t controls, plugin_ui_aspect_t aspect);

int plugin_ui_aspect_control_frame_add(plugin_ui_aspect_t aspect, plugin_ui_control_frame_t frame, uint8_t is_tie);
void plugin_ui_aspect_control_frame_remove(plugin_ui_aspect_t aspect, plugin_ui_control_frame_t frame);
uint8_t plugin_ui_aspect_control_frame_is_in(plugin_ui_aspect_t aspect, plugin_ui_control_frame_t frame);        
void plugin_ui_aspect_control_frame_clear(plugin_ui_aspect_t aspect);
void plugin_ui_aspect_control_frames(plugin_ui_control_frame_it_t control_frames, plugin_ui_aspect_t aspect);

int plugin_ui_aspect_control_action_add(plugin_ui_aspect_t aspect, plugin_ui_control_action_t action, uint8_t is_tie);
void plugin_ui_aspect_control_action_remove(plugin_ui_aspect_t aspect, plugin_ui_control_action_t action);    
uint8_t plugin_ui_aspect_control_action_is_in(plugin_ui_aspect_t aspect, plugin_ui_control_action_t action);        
void plugin_ui_aspect_control_action_clear(plugin_ui_aspect_t aspect);
void plugin_ui_aspect_control_actions(plugin_ui_control_action_it_t control_actions, plugin_ui_aspect_t aspect);

int plugin_ui_aspect_animation_add(plugin_ui_aspect_t aspect, plugin_ui_animation_t animation, uint8_t is_tie);
void plugin_ui_aspect_animation_remove(plugin_ui_aspect_t aspect, plugin_ui_animation_t animation);
uint8_t plugin_ui_aspect_animation_is_in(plugin_ui_aspect_t aspect, plugin_ui_animation_t animation);        
void plugin_ui_aspect_animation_clear(plugin_ui_aspect_t aspect);
void plugin_ui_aspect_animations(plugin_ui_animation_it_t animations, plugin_ui_aspect_t aspect);

#ifdef __cplusplus
}
#endif

#endif

