#ifndef UI_SPRITE_UI_ACTION_NAVIGATION_I_H
#define UI_SPRITE_UI_ACTION_NAVIGATION_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_navigation.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_navigation {
    ui_sprite_ui_module_t m_module;
    char * m_event;
    char * m_condition;
    plugin_ui_navigation_t m_navigation;
};

int ui_sprite_ui_action_navigation_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_navigation_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
