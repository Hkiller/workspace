#ifndef UI_SPRITE_BARRAGE_MASK_H
#define UI_SPRITE_BARRAGE_MASK_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_OBJ_MASK_NAME;

ui_sprite_barrage_mask_t ui_sprite_barrage_mask_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_barrage_mask_free(ui_sprite_barrage_mask_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
