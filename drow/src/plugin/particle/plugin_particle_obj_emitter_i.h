#ifndef UI_PLUGIN_PARTICLE_OBJ_EMITTER_I_H
#define UI_PLUGIN_PARTICLE_OBJ_EMITTER_I_H
#include "cpe/utils/bitarry.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin_particle_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*runtime*/
typedef enum plugin_particle_obj_emitter_state {
    plugin_particle_obj_emitter_state_pendkill, /* kill request from outside */
    plugin_particle_obj_emitter_state_waitstop, /* the simulation end natually */
} plugin_particle_obj_emitter_state_t;

struct plugin_particle_obj_emitter_runtime {
    float m_elapsed_time;     /** used to control the lifetime of the emitter */
    float m_emit_counter;     /** used to calculate how many particles should be spawned in next frame */
    uint8_t m_is_first_frame;      /** if this frame is a first frame after active */
    uint8_t m_is_first_shoot;      /** if this frame is a first shoot of the emitter */
};
    
struct plugin_particle_obj_emitter {
    plugin_particle_obj_t m_obj;
    TAILQ_ENTRY(plugin_particle_obj_emitter) m_next;
    struct dr_data m_addition_data;
    plugin_particle_data_emitter_t m_static_data;
    plugin_particle_obj_emitter_data_t m_runtime_data;
    plugin_particle_obj_emitter_mod_list_t m_mods;
    plugin_particle_obj_mod_data_list_t m_mod_datas;
    plugin_particle_obj_mod_data_t m_cur_mod_data;
    plugin_particle_obj_emitter_lifecircle_t m_lifecircle;
    plugin_particle_obj_emitter_binding_list_t m_binding_particles;
    char * m_name;

    ui_transform m_transform;
    uint8_t m_have_transform;
    
    plugin_particle_obj_plugin_list_t m_plugins;

    uint32_t m_particle_count;
    plugin_particle_obj_particle_list_t m_particles;

    /*runtime*/
    float m_time_scale;
    uint8_t m_flags[1];
    plugin_particle_obj_emitter_use_state_t m_use_state;
    float m_play_counter;     /** counter for play times */
    struct plugin_particle_obj_emitter_runtime m_runtime;
    
    ui_cache_res_t m_texture;           /** the index of the particle texture */
    plugin_particle_obj_emitter_texture_mode_t m_texture_mode;
    ui_vector_2 m_texture_origin_size;
    ui_vector_2 m_texture_size;
    ui_vector_2 m_tile_size;
    
    struct ui_vector_2 m_tex_coord_start;      /** texture coordinates offset */
    struct ui_vector_2 m_tex_coord_scale;      /** texture coordinates scaling */

    ui_vector_2 m_normalized_vtx[4]; /** normailized vertex used to represent a sprite */
};

void plugin_particle_obj_emitter_do_active(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_emitter_do_deactive(plugin_particle_obj_emitter_t emitter);

void plugin_particle_obj_emitter_texture_cache_init(plugin_particle_obj_emitter_t emitter);

void plugin_particle_obj_emitter_real_free(
    plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter);

int8_t plugin_particle_obj_emitter_mod_index(plugin_particle_obj_emitter_t emitter, UI_PARTICLE_MOD const * mod);

void plugin_particle_obj_emitter_runtime_init(plugin_particle_obj_emitter_runtime_t runtime);

float plugin_particle_obj_emitter_curve_sample(plugin_particle_obj_emitter_t obj, uint16_t curve_id, float key);

const char * plugin_particle_obj_emitter_msg(plugin_particle_obj_emitter_t emitter, uint32_t msg);

#ifdef __cplusplus
}
#endif

#endif
