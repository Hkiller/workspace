#ifndef UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE_I_H
#define UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE_I_H
#include "ui/sprite_render/ui_sprite_render_action_obj_bind_value.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_action_obj_bind_value {
    ui_sprite_render_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_setup;
};

int ui_sprite_render_action_obj_bind_value_regist(ui_sprite_render_module_t module);
void ui_sprite_render_action_obj_bind_value_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
