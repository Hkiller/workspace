#ifndef UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_H
#define UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_WAIT_ENTITY_NOT_IN_STATE_NAME;

ui_sprite_basic_wait_entity_not_in_state_t ui_sprite_basic_wait_entity_not_in_state_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_wait_entity_not_in_state_free(ui_sprite_basic_wait_entity_not_in_state_t wait_entity_not_in_state);

const char * ui_sprite_basic_wait_entity_not_in_state_entity(ui_sprite_basic_wait_entity_not_in_state_t entity_exitst);
int ui_sprite_basic_wait_entity_not_in_state_set_entity(ui_sprite_basic_wait_entity_not_in_state_t entity_exitst, const char * entity);

const char * ui_sprite_basic_wait_entity_not_in_state_state(ui_sprite_basic_wait_entity_not_in_state_t entity_exitst);
int ui_sprite_basic_wait_entity_not_in_state_set_state(ui_sprite_basic_wait_entity_not_in_state_t entity_exitst, const char * state);
    
#ifdef __cplusplus
}
#endif

#endif
