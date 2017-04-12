#ifndef UI_SPRITE_BASIC_DEBUG_H
#define UI_SPRITE_BASIC_DEBUG_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_DEBUG_NAME;

ui_sprite_basic_debug_t ui_sprite_basic_debug_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_debug_free(ui_sprite_basic_debug_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
