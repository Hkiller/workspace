#ifndef UI_SPRITE_SPINE_UI_ANIM_BIND_H
#define UI_SPRITE_SPINE_UI_ANIM_BIND_H
#include "ui_sprite_spine_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_UI_ANIM_BIND_NAME;

ui_sprite_spine_ui_anim_bind_t ui_sprite_spine_ui_anim_bind_create(plugin_ui_env_t env);

int ui_sprite_spine_ui_anim_bind_add_root(ui_sprite_spine_ui_anim_bind_t anim_bind, const char * root);
int ui_sprite_spine_ui_anim_bind_set_prefix(ui_sprite_spine_ui_anim_bind_t anim_bind, const char * prefix);
    
#ifdef __cplusplus
}
#endif

#endif
