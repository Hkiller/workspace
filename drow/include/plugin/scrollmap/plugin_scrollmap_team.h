#ifndef PLUGIN_SCROLLMAP_TEAM_H
#define PLUGIN_SCROLLMAP_TEAM_H
#include "render/runtime/ui_runtime_types.h"
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_team_it {
    plugin_scrollmap_team_t (*m_next)(plugin_scrollmap_team_it_t it);
    char m_data[16];
};

plugin_scrollmap_team_t plugin_scrollmap_team_create_from(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, ui_data_src_t src);
plugin_scrollmap_team_t plugin_scrollmap_team_create_from_res(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, const char * res);
void plugin_scrollmap_team_free(plugin_scrollmap_team_t team);

uint16_t plugin_scrollmap_team_id(plugin_scrollmap_team_t team);
plugin_scrollmap_team_t plugin_scrollmap_team_find_by_id(plugin_scrollmap_env_t env, uint16_t team_id);

ui_transform_t plugin_scrollmap_team_transform(plugin_scrollmap_team_t team);
void plugin_scrollmap_team_set_transform(plugin_scrollmap_team_t team, ui_transform_t trans);
    
uint16_t plugin_scrollmap_env_team_count(plugin_scrollmap_env_t env);

int plugin_scrollmap_team_add_name_map(plugin_scrollmap_team_t team, const char * from_type, const char * to_type);
    
#define plugin_scrollmap_team_it_next(__it) ((__it)->m_next)(__it)
    
#ifdef __cplusplus
}
#endif

#endif
