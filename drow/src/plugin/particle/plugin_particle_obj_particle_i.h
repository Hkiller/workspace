#ifndef UI_PLUGIN_PARTICLE_OBJ_PARTICLE_I_H
#define UI_PLUGIN_PARTICLE_OBJ_PARTICLE_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin_particle_obj_emitter_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_particle {
    plugin_particle_obj_emitter_t m_emitter;
    TAILQ_ENTRY(plugin_particle_obj_particle) m_next;
    plugin_particle_obj_plugin_data_list_t m_plugin_datas;
    plugin_particle_obj_emitter_binding_list_t m_binding_emitters;

    float m_time_scale;
    plugin_particle_obj_particle_t m_follow_to;
    uint8_t m_follow_is_tie;
    uint8_t m_follow_angle;    
    uint8_t m_follow_scale;    
    TAILQ_ENTRY(plugin_particle_obj_particle) m_next_for_follow;

    plugin_particle_obj_particle_list_t m_follow_particles;
    
    uint8_t m_disable_mods[4];
    uint32_t m_repeat_count;
    ui_vector_2 m_track_location;
    
    ui_vector_2 m_location;
    ui_vector_2 m_velocity;

    ui_vector_2 m_size;
    uint32_t m_color;

    ui_vector_2 m_base_accel;
    ui_vector_2 m_base_size;
    uint32_t m_base_color;

    float m_spin_init;
    float m_spin_rate;
    
    float m_relative_time;                       /** relative time, range is 0 (==spawn) to 1 (==death) */
    float m_one_over_max_life;                     /** reciprocal of lifetime */
    float m_moved_distance;

    ui_vector_2 m_spawn_scale;                           /** scaling for a world xform mode particle */

    /*for orbit*/
    ui_vector_2 m_orbit;

    union {
        struct {
            uint32_t m_index;                  /** sub tile index for the sprite texture */
        } m_texture_tile;

        struct {
            float m_u;                      /** u offset for scroll animation */
            float m_v;                      /** v offset for scroll animation */
        } m_texture_scroll;
    };
};

void plugin_particle_obj_particle_real_free(
    plugin_particle_module_t module, plugin_particle_obj_particle_t particle);

void plugin_particle_obj_particle_kill(plugin_particle_obj_particle_t particle);

void plugin_particle_obj_particle_calc_auto_dir_up(plugin_particle_obj_particle_t particle);

#ifdef __cplusplus
}
#endif

#endif
