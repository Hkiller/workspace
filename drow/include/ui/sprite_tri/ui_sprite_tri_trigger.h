#ifndef UI_SPRITE_TRI_TRIGGER_H
#define UI_SPRITE_TRI_TRIGGER_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_tri_trigger_type {
    ui_sprite_tri_trigger_type_on_event,
    ui_sprite_tri_trigger_type_on_attr,
} ui_sprite_tri_trigger_type_t;
    
ui_sprite_tri_trigger_t
ui_sprite_tri_trigger_create_on_event(ui_sprite_tri_rule_t rule, const char * event, const char * condition);
ui_sprite_tri_trigger_t ui_sprite_tri_trigger_create_on_attr(ui_sprite_tri_rule_t rule, const char * condition);
ui_sprite_tri_trigger_t ui_sprite_tri_trigger_clone(ui_sprite_tri_rule_t rule, ui_sprite_tri_trigger_t source);
void ui_sprite_tri_trigger_free(ui_sprite_tri_trigger_t trigger);

#ifdef __cplusplus
}
#endif

#endif
