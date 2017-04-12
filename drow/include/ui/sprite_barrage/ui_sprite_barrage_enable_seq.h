#ifndef UI_SPRITE_BARRAGE_ENABLE_SEQ_H
#define UI_SPRITE_BARRAGE_ENABLE_SEQ_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME;

ui_sprite_barrage_enable_seq_t ui_sprite_barrage_enable_seq_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_barrage_enable_seq_free(ui_sprite_barrage_enable_seq_t send_evt);

const char * ui_sprite_barrage_enable_seq_group_name(ui_sprite_barrage_enable_seq_t enable_seq);
int ui_sprite_barrage_enable_seq_set_group_name(ui_sprite_barrage_enable_seq_t enable_seq, const char * name);

#ifdef __cplusplus
}
#endif

#endif
