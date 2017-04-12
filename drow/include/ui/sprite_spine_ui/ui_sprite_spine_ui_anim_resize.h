#ifndef UI_SPRITE_SPINE_UI_ANIM_RESIZE_H
#define UI_SPRITE_SPINE_UI_ANIM_RESIZE_H
#include "ui_sprite_spine_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_UI_ANIM_RESIZE_NAME;

ui_sprite_spine_ui_anim_resize_t ui_sprite_spine_ui_anim_resize_create(plugin_ui_env_t env);
int ui_sprite_spine_ui_anim_resize_add_frame(ui_sprite_spine_ui_anim_resize_t anim_resize, plugin_ui_control_frame_t frame);

#ifdef __cplusplus
}
#endif

#endif
