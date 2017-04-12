#ifndef PLUGIN_SCROLLMAP_SCENE_H
#define PLUGIN_SCROLLMAP_SCENE_H
#include "cpe/utils/memory.h"
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_scrollmap_env_t plugin_scrollmap_env_create(
    plugin_scrollmap_module_t module, plugin_scrollmap_moving_way_t moving_way, ui_vector_2_t base_size);
    
void plugin_scrollmap_env_free(plugin_scrollmap_env_t env);

plugin_scrollmap_moving_way_t plugin_scrollmap_env_moving_way(plugin_scrollmap_env_t env);
ui_vector_2_t plugin_scrollmap_env_base_size(plugin_scrollmap_env_t env);

plugin_scrollmap_resize_policy_t plugin_scrollmap_env_resize_policy_x(plugin_scrollmap_env_t env);
void plugin_scrollmap_env_set_resize_policy_x(plugin_scrollmap_env_t env, plugin_scrollmap_resize_policy_t policy);

plugin_scrollmap_resize_policy_t plugin_scrollmap_env_resize_policy_y(plugin_scrollmap_env_t env);
void plugin_scrollmap_env_set_resize_policy_y(plugin_scrollmap_env_t env, plugin_scrollmap_resize_policy_t policy);

ui_vector_2_t plugin_scrollmap_env_runing_rect(plugin_scrollmap_env_t env);
void plugin_scrollmap_env_set_runing_size(plugin_scrollmap_env_t env, ui_vector_2_t runing_size);
ui_vector_2_t plugin_scrollmap_env_logic_size_adj(plugin_scrollmap_env_t env);

void plugin_scrollmap_env_set_suspend(plugin_scrollmap_env_t env, uint8_t is_suspend);
uint8_t plugin_scrollmap_env_is_suspend(plugin_scrollmap_env_t env);
    
float plugin_scrollmap_env_move_speed(plugin_scrollmap_env_t env);
void plugin_scrollmap_env_set_move_speed(plugin_scrollmap_env_t env, float speed);

ui_rect_t plugin_scrollmap_env_view_pos(plugin_scrollmap_env_t env);

uint8_t plugin_scrollmap_env_debug(plugin_scrollmap_env_t env);
void plugin_scrollmap_env_set_debug(plugin_scrollmap_env_t env, uint8_t is_debug);

void plugin_scrollmap_env_update(plugin_scrollmap_env_t env, float delta_s);

int plugin_scrollmap_env_load_data(
    plugin_scrollmap_env_t env,
    plugin_scrollmap_source_t source, float start_pos);
    
#ifdef __cplusplus
}
#endif

#endif
