#ifndef UI_SPRITE_PARTICLE_CHIPMUNK_WITH_COLLISION_I_H
#define UI_SPRITE_PARTICLE_CHIPMUNK_WITH_COLLISION_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_spine_chipmunk/ui_sprite_spine_chipmunk_with_collision.h"
#include "ui_sprite_spine_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_chipmunk_with_collision {
    ui_sprite_spine_chipmunk_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_prefix;
    char * m_cfg_name_sep;
    char * m_cfg_collision_group;
    char * m_cfg_collision_category;
    char * m_cfg_collision_mask;

    uint32_t m_collision_group;
    uint32_t m_collision_category;
    uint32_t m_collision_mask;
    ui_sprite_render_anim_t m_render_anim;
    ui_sprite_spine_chipmunk_body_list_t m_bodies;
    ui_sprite_spine_chipmunk_body_list_t m_collided_bodies;
};

int ui_sprite_spine_chipmunk_with_collision_regist(ui_sprite_spine_chipmunk_module_t module);
void ui_sprite_spine_chipmunk_with_collision_unregist(ui_sprite_spine_chipmunk_module_t module);

#ifdef __cplusplus
}
#endif

#endif
