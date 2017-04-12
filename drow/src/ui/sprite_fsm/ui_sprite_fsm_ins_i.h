#ifndef UI_SPRITE_FSM_INS_I_H
#define UI_SPRITE_FSM_INS_I_H
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_fsm_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_fsm_ins_list, ui_sprite_fsm_ins) ui_sprite_fsm_ins_list_t;
typedef TAILQ_HEAD(ui_sprite_fsm_state_list, ui_sprite_fsm_state) ui_sprite_fsm_state_list_t;

struct ui_sprite_fsm_ins {
    ui_sprite_fsm_module_t m_module;
    uint16_t m_max_state_id;
    ui_sprite_fsm_ins_t m_parent;
    ui_sprite_fsm_ins_list_t m_childs;
    TAILQ_ENTRY(ui_sprite_fsm_ins) m_next_for_parent;

    ui_sprite_fsm_state_t m_init_state;
    ui_sprite_fsm_state_t m_init_call_state;
    ui_sprite_fsm_state_t m_cur_state;
    ui_sprite_fsm_state_list_t m_states;
};

void ui_sprite_fsm_ins_init(ui_sprite_fsm_ins_t fsm, ui_sprite_fsm_module_t module, ui_sprite_fsm_ins_t parent_fsm);
void ui_sprite_fsm_ins_reinit(ui_sprite_fsm_ins_t fsm);
void ui_sprite_fsm_ins_fini(ui_sprite_fsm_ins_t fsm);
int ui_sprite_fsm_ins_copy(ui_sprite_fsm_ins_t to, ui_sprite_fsm_ins_t from);
int ui_sprite_fsm_ins_set_state(ui_sprite_fsm_ins_t fsm, const char * swith_to, const char * call);

const char * ui_sprite_fsm_ins_path(ui_sprite_fsm_ins_t fsm);

void ui_sprite_fsm_ins_update(ui_sprite_fsm_ins_t fsm, float delta);

int ui_sprite_fsm_ins_enter(ui_sprite_fsm_ins_t fsm);
void ui_sprite_fsm_ins_exit(ui_sprite_fsm_ins_t fsm);

void ui_sprite_fsm_ins_check(ui_sprite_fsm_ins_t fsm);

#ifdef __cplusplus
}
#endif

#endif
