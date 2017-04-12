#ifndef UI_SPRITE_SPINE_UI_ANIM_RESIZE_I_H
#define UI_SPRITE_SPINE_UI_ANIM_RESIZE_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_spine_ui/ui_sprite_spine_ui_anim_resize.h"
#include "ui_sprite_spine_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_ui_anim_resize {
    ui_sprite_spine_ui_module_t m_module;
    plugin_ui_aspect_t m_aspect;
    uint8_t m_follow;
};

int ui_sprite_spine_ui_anim_resize_regist(ui_sprite_spine_ui_module_t module);
void ui_sprite_spine_ui_anim_resize_unregist(ui_sprite_spine_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
