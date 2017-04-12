#ifndef UI_SPRITE_TRI_CONDITION_H
#define UI_SPRITE_TRI_CONDITION_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_tri_condition_t ui_sprite_tri_condition_create(ui_sprite_tri_rule_t rule, const char * type_name);
ui_sprite_tri_condition_t ui_sprite_tri_condition_clone(ui_sprite_tri_rule_t rule, ui_sprite_tri_condition_t source);
void ui_sprite_tri_condition_free(ui_sprite_tri_condition_t condition);

ui_sprite_tri_condition_meta_t ui_sprite_tri_condition_meta(ui_sprite_tri_condition_t condition);
ui_sprite_tri_rule_t ui_sprite_tri_condition_rule(ui_sprite_tri_condition_t condition);
ui_sprite_entity_t ui_sprite_tri_condition_entity(ui_sprite_tri_condition_t condition);

void * ui_sprite_tri_condition_data(ui_sprite_tri_condition_t condition);
ui_sprite_tri_condition_t ui_sprite_tri_condition_from_data(void * data);

#ifdef __cplusplus
}
#endif

#endif
