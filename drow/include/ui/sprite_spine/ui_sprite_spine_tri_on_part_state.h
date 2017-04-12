#ifndef UI_SPRITE_SPINE_TRI_ON_PART_STATE_H
#define UI_SPRITE_SPINE_TRI_ON_PART_STATE_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_TRI_ON_PART_STATE;

ui_sprite_spine_tri_on_part_state_t
ui_sprite_spine_tri_on_part_state_create(ui_sprite_tri_rule_t rule);
    
void ui_sprite_spine_tri_on_part_state_free(ui_sprite_spine_tri_on_part_state_t on_part_state);

plugin_spine_obj_t ui_sprite_spine_tri_on_part_state_obj(ui_sprite_spine_tri_on_part_state_t on_part_state);
void ui_sprite_spine_tri_on_part_state_set_obj(ui_sprite_spine_tri_on_part_state_t on_part_state, plugin_spine_obj_t obj);
    
const char * ui_sprite_spine_tri_on_part_state_part_name(ui_sprite_spine_tri_on_part_state_t on_part_state);
int ui_sprite_spine_tri_on_part_state_set_part_name(ui_sprite_spine_tri_on_part_state_t on_part_state, const char * name);

const char * ui_sprite_spine_tri_on_part_state_part_state(ui_sprite_spine_tri_on_part_state_t on_part_state);
int ui_sprite_spine_tri_on_part_state_set_part_state(ui_sprite_spine_tri_on_part_state_t on_part_state, const char * state);

uint8_t ui_sprite_spine_tri_on_part_state_include_transition(ui_sprite_spine_tri_on_part_state_t on_part_state);
void ui_sprite_spine_tri_on_part_state_set_include_transition(ui_sprite_spine_tri_on_part_state_t on_part_state, uint8_t i);

#ifdef __cplusplus
}
#endif

#endif
