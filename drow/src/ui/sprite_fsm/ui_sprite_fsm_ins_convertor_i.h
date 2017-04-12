#ifndef UI_SPRITE_FSM_INS_CONVERTOR_I_H
#define UI_SPRITE_FSM_INS_CONVERTOR_I_H
#include "cpe/xcalc/xcalc_types.h"
#include "ui_sprite_fsm_ins_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_convertor {
    ui_sprite_fsm_action_t m_action;
    ui_sprite_event_handler_t m_handler;
    const char * m_event;
    const char * m_convert_to;
    const char * m_condition;
    TAILQ_ENTRY(ui_sprite_fsm_convertor) m_next_for_action;
};

void ui_sprite_fsm_convertor_free_all(ui_sprite_fsm_action_t action);

int ui_sprite_fsm_convertor_enter_all(ui_sprite_fsm_action_t action);
void ui_sprite_fsm_convertor_exit_all(ui_sprite_fsm_action_t action);

int ui_sprite_fsm_convertor_enter(ui_sprite_fsm_convertor_t convertor);
void ui_sprite_fsm_convertor_exit(ui_sprite_fsm_convertor_t convertor);

#ifdef __cplusplus
}
#endif

#endif
