#ifndef UI_SPRITE_FSM_INS_TRANSITION_I_H
#define UI_SPRITE_FSM_INS_TRANSITION_I_H
#include "ui_sprite_fsm_ins_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_transition {
    ui_sprite_fsm_state_t m_state;
    ui_sprite_event_handler_t m_handler;
    const char * m_event;
    const char * m_to_state;
    const char * m_call_state;
    const char * m_condition;
    TAILQ_ENTRY(ui_sprite_fsm_transition) m_next_for_state;
};

int ui_sprite_fsm_transition_enter(ui_sprite_fsm_transition_t transition);
void ui_sprite_fsm_transition_exit(ui_sprite_fsm_transition_t transition);

#ifdef __cplusplus
}
#endif

#endif
