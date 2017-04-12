#ifndef PLUGIN_BARRAGE_BULLET_I_H
#define PLUGIN_BARRAGE_BULLET_I_H
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin/barrage/plugin_barrage_bullet.h"
#include "plugin_barrage_emitter_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_bullet {
    plugin_barrage_env_t m_env;
    plugin_barrage_group_t m_group;
    plugin_barrage_emitter_t m_emitter;
    plugin_barrage_bullet_proto_t m_proto;
    plugin_particle_obj_particle_t m_particle;
    uint32_t m_collision_group;
    uint32_t m_collision_category;
    uint32_t m_collision_mask;
    uint32_t m_show_dead_anim_mask;
    plugin_barrage_data_emitter_flip_type_t m_flip_type;
    plugin_barrage_data_t m_carray_data;
    float m_speed_adj;
    plugin_barrage_target_fun_t m_target_fun;
    void * m_target_fun_ctx;
    struct plugin_barrage_trigger_op_list m_trigger_ops;

    TAILQ_ENTRY(plugin_barrage_bullet) m_next_for_env;
    TAILQ_ENTRY(plugin_barrage_bullet) m_next_for_group;    
    TAILQ_ENTRY(plugin_barrage_bullet) m_next_for_emitter;

    plugin_barrage_bullet_state_t m_state;
    cpBody m_body;
    cpPolyShape m_shape;
    BARRAGE_BULLET m_data;
    plugin_barrage_data_bullet_trigger_t m_next_trigger;
    plugin_barrage_data_bullet_trigger_t m_check_triggers;
};

plugin_barrage_bullet_t
plugin_barrage_bullet_create(
    plugin_barrage_emitter_t emitter,
    plugin_barrage_data_bullet_trigger_t frame_trigger,
    plugin_barrage_data_bullet_trigger_t check_triggers);

void plugin_barrage_bullet_remove_emitter(plugin_barrage_bullet_t bullet);

void plugin_barrage_bullet_real_free(plugin_barrage_bullet_t bullet);

void plugin_barrage_bullet_on_collided(plugin_barrage_bullet_t bullet);

uint8_t plugin_barrage_bullet_trigger_check(plugin_barrage_bullet_t bullet, plugin_barrage_data_bullet_trigger_t trigger);
void plugin_barrage_bullet_trigger_do(plugin_barrage_bullet_t bullet, plugin_barrage_data_bullet_trigger_t trigger);

void plugin_barrage_bullet_update_speed_angle(plugin_barrage_bullet_t bullet);
void plugin_barrage_bullet_update_shape(plugin_barrage_bullet_t bullet);

#ifdef __cplusplus
}
#endif

#endif
