#ifndef UI_SPRITE_FSM_COMPONENT_H
#define UI_SPRITE_FSM_COMPONENT_H
#include "ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_FSM_COMPONENT_FSM_NAME;

ui_sprite_fsm_component_fsm_t ui_sprite_fsm_component_create(ui_sprite_entity_t entity);
ui_sprite_fsm_component_fsm_t ui_sprite_fsm_component_find(ui_sprite_entity_t entity);

void ui_sprite_fsm_component_fsm_set_auto_destory(ui_sprite_fsm_component_fsm_t component_fsm, uint8_t auto_destory);
int ui_sprite_fsm_component_fsm_set_state(ui_sprite_fsm_component_fsm_t component_fsm, const char * switch_to, const char * call);

#ifdef __cplusplus
}
#endif

#endif
