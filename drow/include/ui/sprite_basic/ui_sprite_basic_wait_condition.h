#ifndef UI_SPRITE_BASIC_WAIT_CONDITION_H
#define UI_SPRITE_BASIC_WAIT_CONDITION_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_WAIT_CONDITION_NAME;

ui_sprite_basic_wait_condition_t ui_sprite_basic_wait_condition_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_wait_condition_free(ui_sprite_basic_wait_condition_t wait_condition);

const char * ui_sprite_basic_wait_condition_check(ui_sprite_basic_wait_condition_t wait_condition);
int ui_sprite_basic_wait_condition_set_check(ui_sprite_basic_wait_condition_t wait_condition, const char * check);

#ifdef __cplusplus
}
#endif

#endif
