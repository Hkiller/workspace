#ifndef PLUGIN_SCROLLMAP_OBJ_I_H
#define PLUGIN_SCROLLMAP_OBJ_I_H
#include "render/utils/ui_transform.h"
#include "plugin/scrollmap/plugin_scrollmap_obj.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_obj {
    plugin_scrollmap_env_t m_env;
    TAILQ_ENTRY(plugin_scrollmap_obj) m_next_for_env;
    plugin_scrollmap_layer_t m_layer;
    uint8_t m_is_created;
    uint8_t m_is_move_suspend;
    uint8_t m_accept_scale;
    uint8_t m_accept_angle;
    float m_range;
    ui_transform m_transform;
    plugin_scrollmap_obj_move_state_t m_move_state;
    union {
        struct { 
            TAILQ_ENTRY(plugin_scrollmap_obj) m_next;
        } m_move_by_layer;
        struct {
            plugin_scrollmap_team_t m_team;
            union {
                plugin_moving_node_t m_node;
                plugin_particle_obj_particle_t m_particle;
                struct spBone * m_bone;
            };
            TAILQ_ENTRY(plugin_scrollmap_obj) m_next;
        } m_move_by_team;
    };
};

void plugin_scrollmap_obj_real_free(plugin_scrollmap_obj_t obj);
const char * plugin_scrollmap_obj_analize_name(const char * input, char * buf, size_t buf_len, char * * args);
void plugin_scrollmap_obj_unset_move_by_particle_team(plugin_scrollmap_obj_t obj);
    
#ifdef __cplusplus
}
#endif

#endif
