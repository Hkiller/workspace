#ifndef UI_SPRITE_FSM_INS_STATE_I_H
#define UI_SPRITE_FSM_INS_STATE_I_H
#include "ui_sprite_fsm_ins_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_state {
    ui_sprite_fsm_ins_t m_ins;
    uint16_t m_id;
    const char * m_name;
    ui_sprite_fsm_state_t m_return_to;
    ui_sprite_event_t m_enter_event;
    TAILQ_ENTRY(ui_sprite_fsm_state) m_next_for_ins;
    ui_sprite_fsm_transition_list_t m_transitions;
    ui_sprite_fsm_action_list_t m_actions;

    ui_sprite_fsm_action_list_t m_updating_actions;
    ui_sprite_fsm_action_list_t m_waiting_actions;
    ui_sprite_fsm_action_list_t m_runing_actions;
    ui_sprite_fsm_action_list_t m_done_actions;
};

int ui_sprite_fsm_state_enter(ui_sprite_fsm_state_t state);
void ui_sprite_fsm_state_exit(ui_sprite_fsm_state_t state);

void ui_sprite_fsm_state_update(ui_sprite_fsm_state_t fsm_state, float delta);
uint16_t ui_sprite_fsm_state_check_actions(ui_sprite_fsm_state_t fsm_state);
void ui_sprite_fsm_state_process_complete(ui_sprite_fsm_state_t fsm_state);

#ifdef __cplusplus
}
#endif

#endif
