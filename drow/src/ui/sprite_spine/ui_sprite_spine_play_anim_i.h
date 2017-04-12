#ifndef UI_SPRITE_SPINE_PLAY_ANIM_I_H
#define UI_SPRITE_SPINE_PLAY_ANIM_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_anim.h"
#include "plugin/spine/plugin_spine_obj_anim_group.h"
#include "ui/sprite_spine/ui_sprite_spine_play_anim.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_play_anim {
    ui_sprite_spine_module_t m_module;
    char * m_anim_name;
    char * m_anim_def;
    plugin_spine_obj_anim_group_t m_anim_group;
};

int ui_sprite_spine_play_anim_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_play_anim_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
