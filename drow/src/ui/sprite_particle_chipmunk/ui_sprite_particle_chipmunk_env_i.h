#ifndef UI_SPRITE_PARTICLE_CHIPMUNK_ENV_I_H
#define UI_SPRITE_PARTICLE_CHIPMUNK_ENV_I_H
#include "chipmunk/chipmunk_private.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui_sprite_particle_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME;

struct ui_sprite_particle_chipmunk_env {
    ui_sprite_particle_chipmunk_module_t m_module;
    ui_sprite_chipmunk_env_t m_chipmunk_env;
    plugin_chipmunk_env_t m_env;
    uint32_t m_collision_type;
    uint32_t m_obj_collision_type;

    ui_sprite_particle_chipmunk_body_list_t m_collided_bodys;
};

int ui_sprite_particle_chipmunk_env_regist(ui_sprite_particle_chipmunk_module_t module);
void ui_sprite_particle_chipmunk_env_unregist(ui_sprite_particle_chipmunk_module_t module);

ui_sprite_particle_chipmunk_env_t ui_sprite_particle_chipmunk_env_find(ui_sprite_world_t world);

ui_sprite_world_res_t ui_sprite_particle_chipmunk_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
