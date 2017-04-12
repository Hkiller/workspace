#ifndef UI_SPRITE_FSM_INS_ATTR_BINDING_I_H
#define UI_SPRITE_FSM_INS_ATTR_BINDING_I_H
#include "ui_sprite_fsm_ins_action_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_action_attr_binding {
    ui_sprite_attr_monitor_t m_handler;
    TAILQ_ENTRY(ui_sprite_fsm_action_attr_binding) m_next;
};

ui_sprite_fsm_action_attr_binding_t
ui_sprite_fsm_action_attr_binding_create(ui_sprite_fsm_action_t action, ui_sprite_attr_monitor_t handler);
void ui_sprite_fsm_action_attr_binding_free(ui_sprite_fsm_action_t action, ui_sprite_fsm_action_attr_binding_t attr_binding);
void ui_sprite_fsm_action_attr_binding_free_all(ui_sprite_fsm_action_t action);


#ifdef __cplusplus
}
#endif

#endif
