#ifndef UI_PLUGIN_PARTICLE_OBJ_PARTICLE_H
#define UI_PLUGIN_PARTICLE_OBJ_PARTICLE_H
#include "render/utils/ui_vector_2.h"
#include "protocol/render/model/ui_particle_mod.h"
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_particle_it {
    plugin_particle_obj_particle_t (*next)(struct plugin_particle_obj_particle_it * it);
    char m_data[64];
};
    
plugin_particle_obj_particle_t plugin_particle_obj_particle_create(plugin_particle_obj_emitter_t m_emitter);
plugin_particle_obj_particle_t plugin_particle_obj_particle_create_at(plugin_particle_obj_emitter_t emitter, ui_transform_t transform);
void plugin_particle_obj_particle_free(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_free_with_dead_anim_r(plugin_particle_obj_particle_t particle);

plugin_particle_obj_emitter_t plugin_particle_obj_particle_emitter(plugin_particle_obj_particle_t particle);

plugin_particle_obj_particle_t plugin_particle_obj_particle_follow_to(plugin_particle_obj_particle_t particle);
uint8_t plugin_particle_obj_particle_follow_is_tie(plugin_particle_obj_particle_t particle);    
void plugin_particle_obj_particle_set_follow_to(
    plugin_particle_obj_particle_t particle, plugin_particle_obj_particle_t follow_to, uint8_t is_tie, uint8_t angle, uint8_t scale);
void plugin_particle_obj_particle_follow_particles(plugin_particle_obj_particle_it_t it, plugin_particle_obj_particle_t particle);
    
void plugin_particle_obj_particle_calc_transform(plugin_particle_obj_particle_t particle, ui_transform_t transform);
void plugin_particle_obj_particle_calc_base_transform(plugin_particle_obj_particle_t particle, ui_transform_t transform);    

ui_vector_2_t plugin_particle_obj_particle_pos(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_pos(plugin_particle_obj_particle_t particle, ui_vector_2_t pos);

ui_vector_2_t plugin_particle_obj_particle_velocity(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_velocity(plugin_particle_obj_particle_t particle, ui_vector_2_t velocity);

float plugin_particle_obj_particle_moved_distance(plugin_particle_obj_particle_t particle);
    
ui_vector_2_t plugin_particle_obj_particle_base_size(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_base_size(plugin_particle_obj_particle_t particle, ui_vector_2_t size);

ui_vector_2 plugin_particle_obj_particle_base_scale(plugin_particle_obj_particle_t particle);
ui_vector_2 plugin_particle_obj_particle_scale(plugin_particle_obj_particle_t particle);
    
float plugin_particle_obj_particle_spin_init(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_spin_init(plugin_particle_obj_particle_t particle, float spin_init);
float plugin_particle_obj_particle_spin(plugin_particle_obj_particle_t particle);

float plugin_particle_obj_particle_time_scale(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_time_scale(plugin_particle_obj_particle_t particle, float time_scale);
    
ui_vector_2 plugin_particle_obj_particle_world_pos(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_world_pos(plugin_particle_obj_particle_t particle, ui_vector_2_t pos);

void plugin_particle_obj_particle_adj_pos(plugin_particle_obj_particle_t particle, ui_vector_2_t pos);

ui_vector_2_t plugin_particle_obj_particle_track_location(plugin_particle_obj_particle_t particle);
void plugin_particle_obj_particle_set_track_location(plugin_particle_obj_particle_t particle, ui_vector_2_t pos);

int plugin_particle_obj_particle_mod_disable(plugin_particle_obj_particle_t particle, UI_PARTICLE_MOD const * mod);
uint8_t plugin_particle_obj_particle_mod_is_disable(plugin_particle_obj_particle_t particle, UI_PARTICLE_MOD const * mod);

int plugin_particle_obj_particle_trigger_particle(plugin_particle_obj_particle_t particle, const char * emitter_def);

void plugin_particle_obj_emitter_particles(plugin_particle_obj_particle_it_t it, plugin_particle_obj_emitter_t emitter);

#define plugin_particle_obj_particle_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
