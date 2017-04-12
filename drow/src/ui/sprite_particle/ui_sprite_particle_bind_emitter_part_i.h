#ifndef UI_SPRITE_PARTICLE_BIND_EMITTER_PART_I_H
#define UI_SPRITE_PARTICLE_BIND_EMITTER_PART_I_H
#include "plugin/particle/plugin_particle_obj.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_particle/ui_sprite_particle_bind_emitter_part.h"
#include "ui_sprite_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_bind_emitter_part {
    ui_sprite_particle_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_prefix;
    char * m_cfg_scope;
    char * m_cfg_angle_adj;
    uint8_t m_cfg_accept_flip;
    uint8_t m_cfg_accept_scale;
    uint8_t m_cfg_accept_angle;

    float m_angle_adj_rad;
    char * m_anim_name;
    ui_sprite_particle_bind_emitter_part_binding_list_t m_bindings;
};

int ui_sprite_particle_bind_emitter_part_regist(ui_sprite_particle_module_t module);
void ui_sprite_particle_bind_emitter_part_unregist(ui_sprite_particle_module_t module);

struct ui_sprite_particle_bind_emitter_part_anim {
    ui_sprite_particle_bind_emitter_part_t m_owner;
    TAILQ_ENTRY(ui_sprite_particle_bind_emitter_part_binding) m_next;
    ui_sprite_2d_part_t m_part;
    plugin_particle_obj_emitter_t m_emitter;
};

#ifdef __cplusplus
}
#endif

#endif
