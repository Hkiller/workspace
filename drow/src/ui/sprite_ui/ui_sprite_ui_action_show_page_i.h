#ifndef UI_SPRITE_UI_ACTION_SHOW_PAGE_I_H
#define UI_SPRITE_UI_ACTION_SHOW_PAGE_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_show_page.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_show_page {
    ui_sprite_ui_module_t m_module;
    char * m_page_name;
    char * m_before_page_name;
    uint8_t m_force_change;
    plugin_ui_page_t m_page;
    plugin_ui_state_node_t m_show_in_state;
};

int ui_sprite_ui_action_show_page_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_show_page_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
