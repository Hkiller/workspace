#ifndef UI_SPRITE_BASIC_NOOP_H
#define UI_SPRITE_BASIC_NOOP_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_NOOP_NAME;

ui_sprite_basic_noop_t ui_sprite_basic_noop_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_noop_free(ui_sprite_basic_noop_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
