#ifndef UI_SPRITE_SPINE_UI_ANIM_TOGGLE_H
#define UI_SPRITE_SPINE_UI_ANIM_TOGGLE_H
#include "ui_sprite_spine_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_UI_ANIM_TOGGLE_NAME;

ui_sprite_spine_ui_anim_toggle_t ui_sprite_spine_ui_anim_toggle_create(plugin_ui_env_t env);
int ui_sprite_spine_ui_anim_toggle_add_frame(ui_sprite_spine_ui_anim_toggle_t anim_toggle, plugin_ui_control_frame_t frame);

#ifdef __cplusplus
}
#endif

#endif
