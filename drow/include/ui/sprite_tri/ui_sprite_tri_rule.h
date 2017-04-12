#ifndef UI_SPRITE_TRI_RULE_H
#define UI_SPRITE_TRI_RULE_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_tri_rule_t ui_sprite_tri_rule_create(ui_sprite_tri_obj_t obj);
ui_sprite_tri_rule_t ui_sprite_tri_rule_clone(ui_sprite_tri_obj_t obj, ui_sprite_tri_rule_t source);
void ui_sprite_tri_rule_free(ui_sprite_tri_rule_t rule);

uint8_t ui_sprite_tri_rule_is_active(ui_sprite_tri_rule_t rule);
void ui_sprite_tri_rule_set_active(ui_sprite_tri_rule_t rule, uint8_t active);    
    
uint8_t ui_sprite_tri_rule_is_effect(ui_sprite_tri_rule_t rule);
    
#ifdef __cplusplus
}
#endif

#endif
