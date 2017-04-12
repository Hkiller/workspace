#ifndef UI_SPRITE_RENDER_SUSPEND_I_H
#define UI_SPRITE_RENDER_SUSPEND_I_H
#include "ui/sprite_render/ui_sprite_render_suspend.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_suspend {
    ui_sprite_render_module_t m_module;
    char * m_cfg_anim_name;
};

int ui_sprite_render_suspend_regist(ui_sprite_render_module_t module);
void ui_sprite_render_suspend_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
