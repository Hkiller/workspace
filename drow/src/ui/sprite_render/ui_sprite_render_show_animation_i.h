#ifndef UI_SPRITE_RENDER_SHOW_ANIMATION_I_H
#define UI_SPRITE_RENDER_SHOW_ANIMATION_I_H
#include "ui/sprite_render/ui_sprite_render_show_animation.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_show_animation {
    ui_sprite_render_module_t m_module;
    char * m_cfg_res;
    char * m_cfg_group;
    char * m_cfg_name;
    uint32_t m_anim_id;
};

int ui_sprite_render_show_animation_regist(ui_sprite_render_module_t module);
void ui_sprite_render_show_animation_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
