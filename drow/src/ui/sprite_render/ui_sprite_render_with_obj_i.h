#ifndef UI_SPRITE_RENDER_WITH_OBJ_I_H
#define UI_SPRITE_RENDER_WITH_OBJ_I_H
#include "ui/sprite_render/ui_sprite_render_with_obj.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_with_obj {
    ui_sprite_render_module_t m_module;
    char * m_cfg_obj_name;
    char * m_cfg_obj_res;
    ui_runtime_render_obj_t m_render_obj;
};

int ui_sprite_render_with_obj_regist(ui_sprite_render_module_t module);
void ui_sprite_render_with_obj_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
