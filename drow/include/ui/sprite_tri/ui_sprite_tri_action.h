#ifndef UI_SPRITE_TRI_ACTION_H
#define UI_SPRITE_TRI_ACTION_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_tri_action_t ui_sprite_tri_action_create(ui_sprite_tri_rule_t rule, const char * type_name);
ui_sprite_tri_action_t ui_sprite_tri_action_clone(ui_sprite_tri_rule_t rule, ui_sprite_tri_action_t source);
void ui_sprite_tri_action_free(ui_sprite_tri_action_t action);

ui_sprite_tri_action_meta_t ui_sprite_tri_action_meta(ui_sprite_tri_action_t action);
ui_sprite_tri_rule_t ui_sprite_tri_action_rule(ui_sprite_tri_action_t action);
ui_sprite_entity_t ui_sprite_tri_action_entity(ui_sprite_tri_action_t action);
    
ui_sprite_tri_action_trigger_t ui_sprite_tri_action_trigger(ui_sprite_tri_action_t action);
void ui_sprite_tri_action_set_trigger(ui_sprite_tri_action_t action, ui_sprite_tri_action_trigger_t trigger);

void * ui_sprite_tri_action_data(ui_sprite_tri_action_t action);
ui_sprite_tri_action_t ui_sprite_tri_action_from_data(void * data);

#ifdef __cplusplus
}
#endif

#endif
