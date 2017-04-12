#ifndef UI_SPRITE_SPINE_MOVE_ENTITY_H
#define UI_SPRITE_SPINE_MOVE_ENTITY_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_MOVE_ENTITY_NAME;

ui_sprite_spine_move_entity_t ui_sprite_spine_move_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_move_entity_free(ui_sprite_spine_move_entity_t send_evt);

const char * ui_sprite_spine_move_entity_res(ui_sprite_spine_move_entity_t move_entity);
int ui_sprite_spine_move_entity_set_res(ui_sprite_spine_move_entity_t move_entity, const char * res);

const char * ui_sprite_spine_move_entity_node_name(ui_sprite_spine_move_entity_t move_entity);
int ui_sprite_spine_move_entity_set_node_name(ui_sprite_spine_move_entity_t move_entity, const char * node_name);

#ifdef __cplusplus
}
#endif

#endif
