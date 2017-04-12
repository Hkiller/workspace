#ifndef UI_SPRITE_2D_SCALE_H
#define UI_SPRITE_2D_SCALE_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_SCALE_NAME;

ui_sprite_2d_scale_t ui_sprite_2d_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_scale_free(ui_sprite_2d_scale_t scale);

int ui_sprite_2d_scale_set_decorator(ui_sprite_2d_scale_t show_anim, const char * decorator);
int ui_sprite_2d_scale_set_target_scale_x(ui_sprite_2d_scale_t scale, const char * target_scale_x);
int ui_sprite_2d_scale_set_target_scale_y(ui_sprite_2d_scale_t scale, const char * target_scale_y);
int ui_sprite_2d_scale_set_step(ui_sprite_2d_scale_t scale, const char * step);
int ui_sprite_2d_scale_set_duration(ui_sprite_2d_scale_t scale, const char * duration);

#ifdef __cplusplus
}
#endif

#endif
