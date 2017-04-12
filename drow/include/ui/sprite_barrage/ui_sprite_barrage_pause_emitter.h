#ifndef UI_SPRITE_BARRAGE_PAUSE_EMITTER_H
#define UI_SPRITE_BARRAGE_PAUSE_EMITTER_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME;

ui_sprite_barrage_pause_emitter_t ui_sprite_barrage_pause_emitter_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_barrage_pause_emitter_free(ui_sprite_barrage_pause_emitter_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
