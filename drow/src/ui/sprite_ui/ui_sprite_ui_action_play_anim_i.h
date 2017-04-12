#ifndef UI_SPRITE_UI_ACTION_PLAY_ANIM_I_H
#define UI_SPRITE_UI_ACTION_PLAY_ANIM_I_H
#include "ui/sprite_ui/ui_sprite_ui_action_play_anim.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_play_anim {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_control; 
    char * m_cfg_back_res;
    char * m_cfg_down_res;
    char * m_cfg_disable_res;

    char * m_page_name;
    plugin_ui_aspect_t m_aspect;
};

int ui_sprite_ui_action_play_anim_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_play_anim_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
