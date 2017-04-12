#ifndef UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_I_H
#define UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_spine_ui/ui_sprite_spine_ui_action_resize_follow.h"
#include "ui_sprite_spine_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_ui_action_resize_follow {
    ui_sprite_spine_ui_module_t m_module;
    char * m_cfg_control;
    char * m_cfg_res;
    uint32_t m_animation_id;
};

int ui_sprite_spine_ui_action_resize_follow_regist(ui_sprite_spine_ui_module_t module);
void ui_sprite_spine_ui_action_resize_follow_unregist(ui_sprite_spine_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
