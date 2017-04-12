#ifndef UI_SPRITE_BARRAGE_CLEAR_BULLETS_H
#define UI_SPRITE_BARRAGE_CLEAR_BULLETS_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME;

ui_sprite_barrage_clear_bullets_t ui_sprite_barrage_clear_bullets_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_barrage_clear_bullets_free(ui_sprite_barrage_clear_bullets_t send_evt);

const char * ui_sprite_barrage_clear_bullets_group_name(ui_sprite_barrage_clear_bullets_t clear_bullets);
int ui_sprite_barrage_clear_bullets_set_group_name(ui_sprite_barrage_clear_bullets_t clear_bullets, const char * name);

#ifdef __cplusplus
}
#endif

#endif
