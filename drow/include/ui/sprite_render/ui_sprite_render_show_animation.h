#ifndef UI_SPRITE_RENDER_ACTION_SHOW_ANIMATION_H
#define UI_SPRITE_RENDER_ACTION_SHOW_ANIMATION_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME;

ui_sprite_render_show_animation_t ui_sprite_render_show_animation_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_show_animation_free(ui_sprite_render_show_animation_t show_animation);

const char * ui_sprite_render_show_animation_res(ui_sprite_render_show_animation_t show_animation);
void ui_sprite_render_show_animation_set_res(ui_sprite_render_show_animation_t show_animation, const char * res);

const char * ui_sprite_render_show_animation_group(ui_sprite_render_show_animation_t show_animation);
void ui_sprite_render_show_animation_set_group(ui_sprite_render_show_animation_t show_animation, const char * group);

const char * ui_sprite_render_show_animation_name(ui_sprite_render_show_animation_t show_animation);
void ui_sprite_render_show_animation_set_name(ui_sprite_render_show_animation_t show_animation, const char * name);

#ifdef __cplusplus
}
#endif

#endif
