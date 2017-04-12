#ifndef UI_SPRITE_FSM_COMPONENT_I_H
#define UI_SPRITE_FSM_COMPONENT_I_H
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui_sprite_fsm_ins_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_component_fsm {
    struct ui_sprite_fsm_ins m_ins;
    uint8_t m_auto_destory;
};

int ui_sprite_fsm_component_regist(ui_sprite_fsm_module_t module);
void ui_sprite_fsm_component_unregist(ui_sprite_fsm_module_t module);

#ifdef __cplusplus
}
#endif

#endif
