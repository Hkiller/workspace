#ifndef UI_SPRITE_CHIPMUNK_TRI_SCOPE_RENDER_I_H
#define UI_SPRITE_CHIPMUNK_TRI_SCOPE_RENDER_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_tri_scope_render.h"
#include "ui_sprite_chipmunk_tri_scope_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_tri_scope_render {
    ui_sprite_chipmunk_env_t m_env;
};

int ui_sprite_chipmunk_tri_scope_render_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_tri_scope_render_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
