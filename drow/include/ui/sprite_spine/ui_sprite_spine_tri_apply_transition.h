#ifndef UI_SPRITE_SPINE_TRI_APPLY_TRANSITION_H
#define UI_SPRITE_SPINE_TRI_APPLY_TRANSITION_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_TRI_APPLY_TRANSITION;

ui_sprite_spine_tri_apply_transition_t
ui_sprite_spine_tri_apply_transition_create(ui_sprite_tri_rule_t rule);

void ui_sprite_spine_tri_apply_transition_free(ui_sprite_spine_tri_apply_transition_t apply_transition);

plugin_spine_obj_t ui_sprite_spine_tri_apply_transition_obj(ui_sprite_spine_tri_apply_transition_t apply_transition);
void ui_sprite_spine_tri_apply_transition_set_obj(ui_sprite_spine_tri_apply_transition_t apply_transition, plugin_spine_obj_t obj);

const char * ui_sprite_spine_tri_apply_transition_part(ui_sprite_spine_tri_apply_transition_t apply_transition);
int ui_sprite_spine_tri_apply_transition_set_part(ui_sprite_spine_tri_apply_transition_t apply_transition, const char * part);
    
const char * ui_sprite_spine_tri_apply_transition_transition(ui_sprite_spine_tri_apply_transition_t apply_transition);
int ui_sprite_spine_tri_apply_transition_set_transition(ui_sprite_spine_tri_apply_transition_t apply_transition, const char * transition);
    
#ifdef __cplusplus
}
#endif

#endif
