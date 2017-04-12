#ifndef UI_SPRITE_2D_WAIT_SWITCHBACK_H
#define UI_SPRITE_2D_WAIT_SWITCHBACK_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_WAIT_SWITCHBACK_NAME;

ui_sprite_2d_wait_switchback_t ui_sprite_2d_wait_switchback_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_wait_switchback_free(ui_sprite_2d_wait_switchback_t wait_switchback);

uint8_t ui_sprite_2d_wait_switchback_pos(ui_sprite_2d_wait_switchback_t wait_switchback);
void ui_sprite_2d_wait_switchback_set_pos(ui_sprite_2d_wait_switchback_t wait_switchback, uint8_t pos_policy);

uint8_t ui_sprite_2d_wait_switchback_process_x(ui_sprite_2d_wait_switchback_t wait_switchback);
void ui_sprite_2d_wait_switchback_set_process_x(ui_sprite_2d_wait_switchback_t wait_switchback, uint8_t process_x);

uint8_t ui_sprite_2d_wait_switchback_process_y(ui_sprite_2d_wait_switchback_t wait_switchback);
void ui_sprite_2d_wait_switchback_set_process_y(ui_sprite_2d_wait_switchback_t wait_switchback, uint8_t process_y);

#ifdef __cplusplus
}
#endif

#endif
