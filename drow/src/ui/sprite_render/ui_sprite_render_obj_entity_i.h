#ifndef UI_SPRITE_RENDER_OBJ_ENTITY_I_H
#define UI_SPRITE_RENDER_OBJ_ENTITY_I_H
#include "render/model/ui_object_ref.h"
#include "ui/sprite_render/ui_sprite_render_obj_entity.h"
#include "ui_sprite_render_module_i.h"

struct ui_sprite_render_obj_entity {
    ui_sprite_render_module_t m_module;
    char m_world[64];
    char m_entity[64];
};

int ui_sprite_render_obj_entity_regist(ui_sprite_render_module_t module);
void ui_sprite_render_obj_entity_unregist(ui_sprite_render_module_t module);

#endif
