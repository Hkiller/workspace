#ifndef UI_SPRITE_SPINE_CONTROL_ENTITY_H
#define UI_SPRITE_SPINE_CONTROL_ENTITY_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_CONTROL_ENTITY_NAME;

ui_sprite_spine_control_entity_t ui_sprite_spine_control_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_control_entity_free(ui_sprite_spine_control_entity_t control_entity);

#ifdef __cplusplus
}
#endif

#endif
