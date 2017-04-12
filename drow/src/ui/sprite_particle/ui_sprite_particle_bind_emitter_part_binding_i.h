#ifndef UI_SPRITE_PARTICLE_BIND_EMITTER_PART_BINDING_I_H
#define UI_SPRITE_PARTICLE_BIND_EMITTER_PART_BINDING_I_H
#include "ui_sprite_particle_bind_emitter_part_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_bind_emitter_part_binding {
    ui_sprite_particle_bind_emitter_part_t m_owner;
    TAILQ_ENTRY(ui_sprite_particle_bind_emitter_part_binding) m_next;
    uint8_t m_accept_flip;
    uint8_t m_accept_scale;
    uint8_t m_accept_angle;
    float m_angle_adj_rad;
    ui_sprite_2d_part_t m_part;
    plugin_particle_obj_emitter_t m_emitter;
};
    
ui_sprite_particle_bind_emitter_part_binding_t
ui_sprite_particle_bind_emitter_part_binding_create(
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part, ui_sprite_2d_part_t part, plugin_particle_obj_emitter_t emitter);
void ui_sprite_particle_bind_emitter_part_binding_free(ui_sprite_particle_bind_emitter_part_binding_t binding);

void ui_sprite_paritcle_bind_emitter_part_binding_update(
    ui_sprite_particle_bind_emitter_part_binding_t binding, ui_transform_t transform);
    
void ui_sprite_particle_bind_emitter_part_binding_real_free(ui_sprite_particle_bind_emitter_part_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif
