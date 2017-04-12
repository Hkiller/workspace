#ifndef UI_SPRITE_FSM_ACTION_FSM_I_H
#define UI_SPRITE_FSM_ACTION_FSM_I_H
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui_sprite_fsm_ins_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_action_fsm {
    struct ui_sprite_fsm_ins m_ins;
    char * m_cfg_load_from;
    struct dr_data m_data;
};

int ui_sprite_fsm_action_fsm_regist(ui_sprite_fsm_module_t module);
void ui_sprite_fsm_action_fsm_unregist(ui_sprite_fsm_module_t module);

#ifdef __cplusplus
}
#endif

#endif
