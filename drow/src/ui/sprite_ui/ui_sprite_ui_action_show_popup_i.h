#ifndef UI_SPRITE_UI_ACTION_SHOW_POPUP_I_H
#define UI_SPRITE_UI_ACTION_SHOW_POPUP_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_show_popup.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_show_popup {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_popup_name;
    plugin_ui_popup_t m_popup;
};

int ui_sprite_ui_action_show_popup_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_show_popup_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
