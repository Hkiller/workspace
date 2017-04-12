#ifndef UI_SPRITE_CHIPMUNK_GEN_ENTITY_I_H
#define UI_SPRITE_CHIPMUNK_GEN_ENTITY_I_H
#include "ui/sprite_particle/ui_sprite_particle_gen_entity.h"
#include "ui_sprite_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_gen_entity {
    ui_sprite_particle_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_prefix;

    char * m_anim_name;
};

int ui_sprite_particle_gen_entity_regist(ui_sprite_particle_module_t module);
void ui_sprite_particle_gen_entity_unregist(ui_sprite_particle_module_t module);

#ifdef __cplusplus
}
#endif

#endif
