#ifndef UI_SPRITE_UI_ACTION_PHASE_SWITCH_I_H
#define UI_SPRITE_UI_ACTION_PHASE_SWITCH_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_phase_switch.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_phase_switch {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_to;
    char * m_cfg_loading;
};

int ui_sprite_ui_action_phase_switch_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_phase_switch_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
