#ifndef UI_PLUGIN_PARTICLE_OBJ_EMITTER_H
#define UI_PLUGIN_PARTICLE_OBJ_EMITTER_H
#include "protocol/render/model/ui_particle.h"
#include "protocol/render/model/ui_particle_mod.h"
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_emitter_it {
    plugin_particle_obj_emitter_t (*next)(struct plugin_particle_obj_emitter_it * it);
    char m_data[64];
};
    
plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_create(plugin_particle_obj_t obj, plugin_particle_data_emitter_t data);
plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_clone(plugin_particle_obj_t obj, plugin_particle_obj_emitter_t o);
    
void plugin_particle_obj_emitter_free(plugin_particle_obj_emitter_t emitter);

plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_find(plugin_particle_obj_t obj, const char * emitter_name);

void plugin_particle_obj_emitter_set_lifecircle(
    plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_lifecircle_t lifecircle);    
plugin_particle_obj_emitter_lifecircle_t plugin_particle_obj_emitter_lifecircle(plugin_particle_obj_emitter_t emitter);
    
plugin_particle_obj_t plugin_particle_obj_emitter_obj(plugin_particle_obj_emitter_t emitter);

const char * plugin_particle_obj_emitter_name(plugin_particle_obj_emitter_t emitter);
int plugin_particle_obj_emitter_set_name(plugin_particle_obj_emitter_t emitter, const char * name);

plugin_particle_data_emitter_t plugin_particle_obj_emitter_static_data(plugin_particle_obj_emitter_t emitter);
UI_PARTICLE_EMITTER const * plugin_particle_obj_emitter_data_r(plugin_particle_obj_emitter_t emitter);
UI_PARTICLE_EMITTER * plugin_particle_obj_emitter_data_w(plugin_particle_obj_emitter_t emitter);

UI_PARTICLE_MOD const * plugin_particle_obj_emitter_find_mod_r(plugin_particle_obj_emitter_t emitter, uint8_t mod_type);
UI_PARTICLE_MOD * plugin_particle_obj_emitter_find_mod_w(plugin_particle_obj_emitter_t emitter, uint8_t mod_type);
UI_PARTICLE_MOD * plugin_particle_obj_emitter_check_create_mod(plugin_particle_obj_emitter_t emitter, uint8_t mod_type);

float plugin_particle_obj_emitter_real_spawn_rate(plugin_particle_obj_emitter_t emitter);
    
int plugin_particle_obj_emitter_reset(plugin_particle_obj_emitter_t emitter);

dr_data_t plugin_particle_obj_emitter_addition_data(plugin_particle_obj_emitter_t emitter);
int plugin_particle_obj_emitter_set_addition_data(plugin_particle_obj_emitter_t emitter, dr_data_t data);

uint8_t plugin_particle_obj_emitter_is_closing(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_emitter_set_close(plugin_particle_obj_emitter_t emitter, uint8_t is_closing);
    
float plugin_particle_obj_emitter_time_scale(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_emitter_set_tile_scale(plugin_particle_obj_emitter_t emitter, float time_scale);
    
plugin_particle_obj_emitter_use_state_t plugin_particle_obj_emitter_use_state(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_emitter_set_use_state(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_use_state_t use_state);

int plugin_particle_obj_emitter_set_attr(plugin_particle_obj_emitter_t emitter, const char * attr_name, dr_value_t attr_value);
int plugin_particle_obj_emitter_set_attr_by_str(plugin_particle_obj_emitter_t emitter, const char * attr_name, const char * attr_value);
int plugin_particle_obj_emitter_get_attr(plugin_particle_obj_emitter_t emitter, const char * attr_name, dr_value_t attr_value);

int plugin_particle_obj_emitter_spawn_at_world(plugin_particle_obj_emitter_t emitter, ui_transform_t trans, uint16_t gen_count);
int plugin_particle_obj_emitter_spawn_at_local(plugin_particle_obj_emitter_t emitter, ui_transform_t trans, uint16_t gen_count);
int plugin_particle_obj_emitter_spawn(plugin_particle_obj_emitter_t emitter, uint16_t gen_count);

ui_transform_t plugin_particle_obj_emitter_transform(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_emitter_set_transform(plugin_particle_obj_emitter_t emitter, ui_transform_t transform);

ui_vector_2_t plugin_particle_obj_emitter_texture_size(plugin_particle_obj_emitter_t emitter);
ui_vector_2_t plugin_particle_obj_emitter_tile_size(plugin_particle_obj_emitter_t emitter);
ui_vector_2_t plugin_particle_obj_emitter_normalized_vtx_4(plugin_particle_obj_emitter_t emitter); /*lb, lt, rt, rb*/

int plugin_particle_obj_emitter_set_mod_track_location(plugin_particle_obj_emitter_t emitter, uint8_t mod_type, ui_vector_2_t pos);

void plugin_particle_obj_emitter_clear_particles(plugin_particle_obj_emitter_t emitter, uint8_t show_dead_anim);

void plugin_particle_obj_emitters(plugin_particle_obj_emitter_it_t it, plugin_particle_obj_t obj);

const char * plugin_particle_obj_emitter_texture(plugin_particle_obj_emitter_t emitter);    
const char * plugin_particle_obj_emitter_user_text(plugin_particle_obj_emitter_t emitter);
const char * plugin_particle_obj_emitter_group(plugin_particle_obj_emitter_t emitter);
const char * plugin_particle_obj_emitter_dead_anim(plugin_particle_obj_emitter_t emitter);
    
#define plugin_particle_obj_emitter_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

