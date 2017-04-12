#ifndef UI_SPRITE_RENDER_OBJ_WORLD_I_H
#define UI_SPRITE_RENDER_OBJ_WORLD_I_H
#include "render/model/ui_object_ref.h"
#include "ui/sprite_render/ui_sprite_render_obj_world.h"
#include "ui_sprite_render_module_i.h"

struct ui_sprite_render_obj_world {
    ui_sprite_render_module_t m_module;
    ui_sprite_render_env_t m_env;
    TAILQ_ENTRY(ui_sprite_render_obj_world) m_next_for_env;
    uint8_t m_control_tick;
    uint8_t m_sync_transform;
};

int ui_sprite_render_obj_world_regist(ui_sprite_render_module_t module);
void ui_sprite_render_obj_world_unregist(ui_sprite_render_module_t module);

#endif
