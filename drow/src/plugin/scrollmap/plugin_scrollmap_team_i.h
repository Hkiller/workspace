#ifndef PLUGIN_SCROLLMAP_TEAM_I_H
#define PLUGIN_SCROLLMAP_TEAM_I_H
#include "render/utils/ui_transform.h"
#include "plugin/moving/plugin_moving_control.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/scrollmap/plugin_scrollmap_team.h"
#include "plugin_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_scrollmap_team_type {
    plugin_scrollmap_team_type_moving,
    plugin_scrollmap_team_type_particle,
    plugin_scrollmap_team_type_spine,
} plugin_scrollmap_team_type_t;

struct plugin_scrollmap_team {
    plugin_scrollmap_env_t m_env;
    plugin_scrollmap_team_type_t m_type;
    ui_transform m_transform;
    union {
        struct {
            plugin_moving_control_t m_control;
        } m_moving;
        struct {
            plugin_particle_obj_t m_obj;
        } m_particle;
        struct {
            plugin_spine_obj_t m_obj;
        } m_spine;
    };
    TAILQ_ENTRY(plugin_scrollmap_team) m_next_for_env;
    plugin_scrollmap_layer_t m_layer;
    uint16_t m_team_id;
    plugin_scrollmap_obj_list_t m_members;
    plugin_scrollmap_obj_type_map_list_t m_obj_type_maps;
};

plugin_scrollmap_team_t plugin_scrollmap_team_create_i(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer);
void plugin_scrollmap_team_real_free(plugin_scrollmap_team_t team);

void plugin_scrollmap_team_update(plugin_scrollmap_env_t env, plugin_scrollmap_team_t team, float delta_s);

int plugin_scrollmap_team_init_from_moving(plugin_scrollmap_team_t team, ui_data_src_t src, plugin_moving_plan_t plan);
void plugin_scrollmap_team_fini_moving(plugin_scrollmap_team_t team);
void plugin_scrollmap_team_update_moving(plugin_scrollmap_team_t team, float delta);
    
int plugin_scrollmap_team_init_from_particle(plugin_scrollmap_team_t team, ui_data_src_t src, plugin_particle_data_t particle_data);
void plugin_scrollmap_team_fini_particle(plugin_scrollmap_team_t team);
void plugin_scrollmap_team_update_particle(plugin_scrollmap_team_t team, float delta);
    
int plugin_scrollmap_team_init_from_spine(plugin_scrollmap_team_t team, ui_data_src_t src, plugin_spine_data_skeleton_t skeleton_data);
void plugin_scrollmap_team_fini_spine(plugin_scrollmap_team_t team);
void plugin_scrollmap_team_update_spine(plugin_scrollmap_team_t team, float delta);

#ifdef __cplusplus
}
#endif

#endif
