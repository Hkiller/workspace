#ifndef UI_SPRITE_SPINE_UI_ANIM_BUTTON_H
#define UI_SPRITE_SPINE_UI_ANIM_BUTTON_H
#include "ui_sprite_spine_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_UI_ANIM_BUTTON_NAME;

ui_sprite_spine_ui_anim_button_t ui_sprite_spine_ui_anim_button_create(plugin_ui_env_t env);

int ui_sprite_spine_ui_anim_button_add_frame(ui_sprite_spine_ui_anim_button_t anim_button, plugin_ui_control_frame_t frame);

int ui_sprite_spine_ui_anim_button_add_control(
    ui_sprite_spine_ui_anim_button_t anim_button, plugin_ui_control_t control);
    
int ui_sprite_spine_ui_anim_button_set_obj(ui_sprite_spine_ui_anim_button_t button, const char * obj);
    
int ui_sprite_spine_ui_anim_button_set_part(ui_sprite_spine_ui_anim_button_t button, const char * part);
int ui_sprite_spine_ui_anim_button_set_up(ui_sprite_spine_ui_anim_button_t button, const char * up);
int ui_sprite_spine_ui_anim_button_set_down(ui_sprite_spine_ui_anim_button_t button, const char * down);
    
#ifdef __cplusplus
}
#endif

#endif
