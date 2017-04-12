#ifndef UI_SPRITE_UI_ACTION_GUARD_PACKAGE_I_H
#define UI_SPRITE_UI_ACTION_GUARD_PACKAGE_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_guard_package.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_guard_package {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_package;

    plugin_package_package_t m_package;
};

ui_sprite_ui_action_guard_package_t ui_sprite_ui_action_guard_package_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_guard_package_free(ui_sprite_ui_action_guard_package_t guard_package);
    
int ui_sprite_ui_action_guard_package_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_guard_package_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
