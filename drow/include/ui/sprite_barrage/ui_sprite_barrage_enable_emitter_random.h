#ifndef UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_H
#define UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME;

ui_sprite_barrage_enable_emitter_random_t ui_sprite_barrage_enable_emitter_random_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_barrage_enable_emitter_random_free(ui_sprite_barrage_enable_emitter_random_t send_evt);

const char * ui_sprite_barrage_enable_emitter_random_group_name(ui_sprite_barrage_enable_emitter_random_t enable_emitter_random);
int ui_sprite_barrage_enable_emitter_random_set_group_name(ui_sprite_barrage_enable_emitter_random_t enable_emitter_random, const char * name);

#ifdef __cplusplus
}
#endif

#endif
