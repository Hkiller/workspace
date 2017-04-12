#ifndef UI_SPRITE_CHIPMUNK_CHIPMUNK_WITH_COLLISION_I_H
#define UI_SPRITE_CHIPMUNK_CHIPMUNK_WITH_COLLISION_I_H
#include "ui/sprite_particle_chipmunk/ui_sprite_particle_chipmunk_with_collision.h"
#include "ui_sprite_particle_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_chipmunk_with_collision {
    ui_sprite_particle_chipmunk_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_prefix;
    char * m_cfg_collision_group;
    char * m_cfg_collision_category;
    char * m_cfg_collision_mask;
    char * m_cfg_collision_event;

    ui_sprite_particle_chipmunk_env_t m_chipmunk_env;
    char * m_anim_name;
    uint32_t m_collision_group;
    uint32_t m_collision_category;
    uint32_t m_collision_mask;
};

int ui_sprite_particle_chipmunk_with_collision_regist(ui_sprite_particle_chipmunk_module_t module);
void ui_sprite_particle_chipmunk_with_collision_unregist(ui_sprite_particle_chipmunk_module_t module);

#ifdef __cplusplus
}
#endif

#endif
