#ifndef UI_SPRITE_FSM_INS_ACTION_I_H
#define UI_SPRITE_FSM_INS_ACTION_I_H
#include "ui_sprite_fsm_ins_state_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_fsm_action_event_binding * ui_sprite_fsm_action_event_binding_t;
typedef TAILQ_HEAD(ui_sprite_fsm_action_event_binding_list, ui_sprite_fsm_action_event_binding) ui_sprite_fsm_action_event_binding_list_t;

typedef struct ui_sprite_fsm_action_attr_binding * ui_sprite_fsm_action_attr_binding_t;
typedef TAILQ_HEAD(ui_sprite_fsm_action_attr_binding_list, ui_sprite_fsm_action_attr_binding) ui_sprite_fsm_action_attr_binding_list_t;

struct ui_sprite_fsm_action {
    ui_sprite_fsm_state_t m_state;
    ui_sprite_fsm_action_meta_t m_meta;
    const char * m_name;
    ui_sprite_event_t m_addition_event; /*for save convertor origin event*/
    ui_sprite_fsm_convertor_list_t m_convertors;
    ui_sprite_fsm_action_life_circle_t m_life_circle;
    float m_duration;
    char * m_duration_calc;
    char * m_condition;
    char * m_work;
    float m_runing_time;
    uint8_t m_apply_enter_evt;
    uint8_t m_is_update;
    TAILQ_ENTRY(ui_sprite_fsm_action) m_next_for_state;
    TAILQ_ENTRY(ui_sprite_fsm_action) m_next_for_work;
    TAILQ_ENTRY(ui_sprite_fsm_action) m_next_for_update;

    ui_sprite_fsm_action_t m_follow_to;
    ui_sprite_fsm_action_list_t m_followers;
    TAILQ_ENTRY(ui_sprite_fsm_action) m_next_for_follow;

    ui_sprite_fsm_action_event_binding_list_t m_event_bindings;
    ui_sprite_fsm_action_attr_binding_list_t m_attr_bindings;
    ui_sprite_fsm_action_state_t m_runing_state;
};

int ui_sprite_fsm_action_enter(ui_sprite_fsm_action_t op_action);
void ui_sprite_fsm_action_exit(ui_sprite_fsm_action_t op_action);
int ui_sprite_fsm_action_check_do_enter(ui_sprite_fsm_action_t op_action);

ui_sprite_event_t ui_sprite_fsm_action_build_event(
    ui_sprite_fsm_action_t op_action, mem_allocrator_t alloc, const char * def, dr_data_source_t data_source);

void ui_sprite_fsm_action_set_runing_state(ui_sprite_fsm_action_t op_action, ui_sprite_fsm_action_state_t runing_state);

#ifdef __cplusplus
}
#endif

#endif
