#ifndef UI_SPRITE_RENDER_LOCK_ON_SCREEN_H
#define UI_SPRITE_RENDER_LOCK_ON_SCREEN_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME;

ui_sprite_render_lock_on_screen_t ui_sprite_render_lock_on_screen_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_lock_on_screen_free(ui_sprite_render_lock_on_screen_t shake);

float ui_sprite_render_lock_on_screen_max_speed(ui_sprite_render_lock_on_screen_t lock_on_screen);
void ui_sprite_render_lock_on_screen_set_max_speed(ui_sprite_render_lock_on_screen_t lock_on_screen, float max_speed);

float ui_sprite_render_lock_on_screen_max_speed(ui_sprite_render_lock_on_screen_t lock_on_screen);
void ui_sprite_render_lock_on_screen_set_max_speed(ui_sprite_render_lock_on_screen_t lock_on_screen, float max_speed);

int ui_sprite_render_lock_on_screen_set_decorator(ui_sprite_render_lock_on_screen_t lock_on_screen, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
