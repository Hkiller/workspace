#ifndef UI_SPRITE_CHIPMUNK_CHIPMUNK_BODY_I_H
#define UI_SPRITE_CHIPMUNK_CHIPMUNK_BODY_I_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_particle_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_particle_chipmunk_body_state {
    ui_sprite_particle_chipmunk_body_state_active,
    ui_sprite_particle_chipmunk_body_state_colliede,
} ui_sprite_particle_chipmunk_body_state_t;
    
struct ui_sprite_particle_chipmunk_body {
    ui_sprite_particle_chipmunk_with_collision_t m_with_collision;
    TAILQ_ENTRY(ui_sprite_particle_chipmunk_body) m_next_for_env;
    ui_sprite_particle_chipmunk_body_state_t m_state;
    ui_vector_2 m_scale;
    cpBody m_body;
    cpPolyShape m_shape;
};

/*return need remove*/
uint8_t ui_sprite_particle_chipmunk_body_on_collided(ui_sprite_particle_chipmunk_body_t body, ui_vector_2_t pt);

int ui_sprite_particle_chipmunk_body_init(void * ctx, plugin_particle_obj_plugin_data_t data);
void ui_sprite_particle_chipmunk_body_fini(void * ctx, plugin_particle_obj_plugin_data_t data);
void ui_sprite_particle_chipmunk_body_update(void * ctx, plugin_particle_obj_plugin_data_t data);

void ui_sprite_particle_chipmunk_body_free(ui_sprite_particle_chipmunk_body_t body);
    
#ifdef __cplusplus
}
#endif

#endif
