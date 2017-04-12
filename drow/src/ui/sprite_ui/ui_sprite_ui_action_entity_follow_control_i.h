#ifndef UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_I_H
#define UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_entity_follow_control.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_entity_follow_control {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_control;
    uint8_t m_cfg_base_policy;

    char * m_control;
};

int ui_sprite_ui_action_entity_follow_control_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_entity_follow_control_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
