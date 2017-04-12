#ifndef UI_SPRITE_SPINE_UI_ANIM_TOGGLE_I_H
#define UI_SPRITE_SPINE_UI_ANIM_TOGGLE_I_H
#include "ui/sprite_spine_ui/ui_sprite_spine_ui_anim_toggle.h"
#include "ui_sprite_spine_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_ui_anim_toggle {
    ui_sprite_spine_ui_module_t m_module;
    plugin_ui_aspect_t m_aspect;
    char * m_obj;
    char * m_part;
    char * m_on;
    char * m_off;
    uint8_t m_need_process;
};

int ui_sprite_spine_ui_anim_toggle_regist(ui_sprite_spine_ui_module_t module);
void ui_sprite_spine_ui_anim_toggle_unregist(ui_sprite_spine_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
