#ifndef PLUGIN_SCROLLMAP_OBJ_H
#define PLUGIN_SCROLLMAP_OBJ_H
#include "plugin/moving/plugin_moving_types.h"
#include "plugin/particle/plugin_particle_types.h"
#include "plugin/spine/plugin_spine_types.h"
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*obj factory*/
int plugin_scrollmap_env_set_obj_factory(
    plugin_scrollmap_env_t env,
    void * ctx, uint32_t obj_capacity,
    plugin_scrollmap_obj_name_fun_t name,
    plugin_scrollmap_obj_on_init_fun_t on_init,
    plugin_scrollmap_obj_on_update_fun_t on_update,
    plugin_scrollmap_obj_on_event_fun_t on_event,
    plugin_scrollmap_obj_on_destory_fun_t on_destory);

void plugin_scrollmap_env_clear_obj_factory(plugin_scrollmap_env_t env, void * ctx);
    
/*obj*/
typedef enum plugin_scrollmap_obj_move_state {
    plugin_scrollmap_obj_move_free = 1
    , plugin_scrollmap_obj_move_by_layer = 2
    , plugin_scrollmap_obj_move_by_team = 3
} plugin_scrollmap_obj_move_state_t;
    
plugin_scrollmap_obj_t plugin_scrollmap_obj_create(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, ui_transform_t transform);
void plugin_scrollmap_obj_free(plugin_scrollmap_obj_t obj);

plugin_scrollmap_layer_t plugin_scrollmap_obj_layer(plugin_scrollmap_obj_t obj);
void plugin_scrollmap_obj_set_layer(plugin_scrollmap_obj_t obj, plugin_scrollmap_layer_t layer);

plugin_scrollmap_obj_move_state_t plugin_scrollmap_obj_move_state(plugin_scrollmap_obj_t obj);
void * plugin_scrollmap_obj_data(plugin_scrollmap_obj_t obj);

ui_transform_t plugin_scrollmap_obj_transform(plugin_scrollmap_obj_t obj);
void plugin_scrollmap_obj_set_transform(plugin_scrollmap_obj_t obj, ui_transform_t transform);

float plugin_scrollmap_obj_range(plugin_scrollmap_obj_t obj);
void plugin_scrollmap_obj_set_range(plugin_scrollmap_obj_t obj, float range);

uint8_t plugin_scrollmap_obj_is_created(plugin_scrollmap_obj_t obj);
int plugin_scrollmap_obj_do_create(plugin_scrollmap_obj_t obj, const char * obj_type, const char * args);
void plugin_scrollmap_obj_do_remove(plugin_scrollmap_obj_t obj);

uint8_t plugin_scrollmap_obj_is_move_suspend(plugin_scrollmap_obj_t obj);
void plugin_scrollmap_obj_set_move_suspend(plugin_scrollmap_obj_t obj, uint8_t move_suspend);

void plugin_scrollmap_obj_set_move_free(plugin_scrollmap_obj_t obj);
    
void plugin_scrollmap_obj_set_move_by_layer(plugin_scrollmap_obj_t obj);
    
plugin_scrollmap_team_t plugin_scrollmap_obj_team(plugin_scrollmap_obj_t obj);
    
int plugin_scrollmap_obj_set_move_by_moving_team(
    plugin_scrollmap_obj_t obj, plugin_scrollmap_team_t team,
    plugin_moving_plan_node_t plan_node, uint8_t loop_count);

int plugin_scrollmap_obj_set_move_by_particle_team(
    plugin_scrollmap_obj_t obj, plugin_scrollmap_team_t team,
    plugin_particle_obj_particle_t particle);

int plugin_scrollmap_obj_set_move_by_spine_team(
    plugin_scrollmap_obj_t obj, plugin_scrollmap_team_t team,
    struct spBone * bone);
    
/*obj_it*/
struct plugin_scrollmap_obj_it {
    plugin_scrollmap_obj_t (*m_next)(plugin_scrollmap_obj_it_t it);
    char m_data[16];
};

#define plugin_scrollmap_obj_it_next(__it) ((__it)->m_next)(__it)
    
#ifdef __cplusplus
}
#endif

#endif
