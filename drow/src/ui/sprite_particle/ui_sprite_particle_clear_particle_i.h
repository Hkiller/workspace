#ifndef UI_SPRITE_PARTICLE_CLEAR_I_H
#define UI_SPRITE_PARTICLE_CLEAR_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "ui/sprite_particle/ui_sprite_particle_clear_particle.h"
#include "ui_sprite_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_clear_particle {
    ui_sprite_particle_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_prefix;
    char * m_cfg_show_dead_anim;
};

int ui_sprite_particle_clear_particle_regist(ui_sprite_particle_module_t module);
void ui_sprite_particle_clear_particle_unregist(ui_sprite_particle_module_t module);

#ifdef __cplusplus
}
#endif

#endif
