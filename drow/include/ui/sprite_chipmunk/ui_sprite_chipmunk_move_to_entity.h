#ifndef UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_H
#define UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME;

ui_sprite_chipmunk_move_to_entity_t ui_sprite_chipmunk_move_to_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_move_to_entity_free(ui_sprite_chipmunk_move_to_entity_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
