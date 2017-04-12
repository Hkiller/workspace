#ifndef UI_SPRITE_RENDER_ENTITY_RENDER_OBJ_CREATOR_I_H
#define UI_SPRITE_RENDER_ENTITY_RENDER_OBJ_CREATOR_I_H
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_render_entity_render_obj_creator_regist(ui_sprite_render_module_t module);
void ui_sprite_render_entity_render_obj_creator_unregist(ui_sprite_render_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
