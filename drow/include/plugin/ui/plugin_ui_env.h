#ifndef DROW_PLUGIN_UI_ENV_H
#define DROW_PLUGIN_UI_ENV_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_env_t plugin_ui_env_create(plugin_ui_module_t module);
void plugin_ui_env_free(plugin_ui_env_t env);

void plugin_ui_env_clear_runtime(plugin_ui_env_t env);

plugin_ui_env_t plugin_ui_env_first(plugin_ui_module_t module);
    
gd_app_context_t plugin_ui_env_app(plugin_ui_env_t env);
plugin_ui_module_t plugin_ui_env_module(plugin_ui_env_t env);
ui_cache_manager_t plugin_ui_env_cache_mgr(plugin_ui_env_t env);
ui_data_mgr_t plugin_ui_env_data_mgr(plugin_ui_env_t env);
plugin_package_module_t plugin_ui_env_package_mgr(plugin_ui_env_t env);
    
void plugin_ui_env_set_backend(plugin_ui_env_t env, plugin_ui_env_backend_t backend);

uint8_t plugin_ui_env_debug(plugin_ui_env_t env);
void plugin_ui_env_set_debug(plugin_ui_env_t env, uint8_t debug);

uint8_t plugin_ui_env_accept_multi_touch(plugin_ui_env_t env);
void plugin_ui_env_set_accept_multi_touch(plugin_ui_env_t env, uint8_t accept_multi_touch);

plugin_ui_screen_resize_policy_t plugin_ui_env_screen_resize_policy(plugin_ui_env_t env);
void plugin_ui_env_set_screen_resize_policy(plugin_ui_env_t env, plugin_ui_screen_resize_policy_t policy);

ui_vector_2_t plugin_ui_env_runtime_sz(plugin_ui_env_t env);
void plugin_ui_env_set_runtime_sz(plugin_ui_env_t env, ui_vector_2_t sz);

ui_vector_2_t plugin_ui_env_origin_sz(plugin_ui_env_t env);
void plugin_ui_env_set_origin_sz(plugin_ui_env_t env, ui_vector_2_t sz);

ui_vector_2_t plugin_ui_env_screen_adj(plugin_ui_env_t env);

uint32_t plugin_ui_env_page_count(plugin_ui_env_t env);
uint32_t plugin_ui_env_page_visiable_count(plugin_ui_env_t env);
uint32_t plugin_ui_env_control_count(plugin_ui_env_t env);
uint32_t plugin_ui_env_control_free_count(plugin_ui_env_t env);
    
void plugin_ui_env_pages(plugin_ui_env_t env, plugin_ui_page_it_t page_it);
void plugin_ui_env_visiable_pages(plugin_ui_env_t env, plugin_ui_page_it_t page_it);
void plugin_ui_env_phase_nodes(plugin_ui_env_t env, plugin_ui_phase_node_it_t phase_node_it);
    
void plugin_ui_env_popups(plugin_ui_env_t env, plugin_ui_popup_it_t popup_it);

void plugin_ui_env_update(plugin_ui_env_t env, float delta);
void plugin_ui_env_render(plugin_ui_env_t env, ui_runtime_render_t ctx);
    
plugin_ui_control_t plugin_ui_env_focus_control(plugin_ui_env_t env);
void plugin_ui_env_set_focus_control(plugin_ui_env_t env, plugin_ui_control_t ctrl);

plugin_ui_control_t plugin_ui_env_float_control(plugin_ui_env_t env);
void plugin_ui_env_set_float_control(plugin_ui_env_t env, plugin_ui_control_t ctrl);
plugin_ui_control_t plugin_ui_env_find_float_control(plugin_ui_env_t env, ui_vector_2_t pt);
    
ui_string_table_t plugin_ui_env_string_table(plugin_ui_env_t env);
    
/*touch process*/
uint8_t plugin_ui_env_accept_input(plugin_ui_env_t env);
void plugin_ui_env_set_accept_input(plugin_ui_env_t env, uint8_t accept);

float plugin_ui_env_long_push_span(plugin_ui_env_t env);    
void plugin_ui_env_set_long_push_span(plugin_ui_env_t env, float span);

uint8_t plugin_ui_env_process_touch_down(plugin_ui_env_t env, int32_t pointer, ui_vector_2_t pt);
uint8_t plugin_ui_env_process_touch_move(plugin_ui_env_t env, int32_t pointer, ui_vector_2_t pt);
uint8_t plugin_ui_env_process_touch_rise(plugin_ui_env_t env, int32_t pointer, ui_vector_2_t pt);    

/*event */
void plugin_ui_env_send_event(plugin_ui_env_t env, LPDRMETA meta, void * data, uint32_t data_size);
void plugin_ui_env_build_and_send_event(plugin_ui_env_t env, const char * def, dr_data_source_t data_source);

/*double click*/
float plugin_ui_env_double_click_span(plugin_ui_env_t env);
void plugin_ui_env_double_click_set_span(plugin_ui_env_t env, float span);
    
/*phase */
int plugin_ui_env_set_init_phase(
    plugin_ui_env_t env, const char * init_phase_name, const char * inint_call_phase);
plugin_ui_phase_t plugin_ui_env_init_phase(plugin_ui_env_t env);
plugin_ui_phase_t plugin_ui_env_init_call_phase(plugin_ui_env_t env);    

int plugin_ui_env_phase_call(
    plugin_ui_env_t env, plugin_ui_phase_t phase,
    plugin_ui_phase_t loading_phase, uint8_t loading_auto_complete,
    plugin_ui_phase_t  back_phase, uint8_t back_auto_complete,
    dr_data_t data);
int plugin_ui_env_phase_call_by_name(
    plugin_ui_env_t env, const char * phase_name, const char * loading_phase_name, const char * back_phase_name,
    dr_data_t data);

int plugin_ui_env_phase_switch(
    plugin_ui_env_t env, plugin_ui_phase_t phase,
    plugin_ui_phase_t loading_phase, uint8_t loading_auto_complete,
    dr_data_t data);
int plugin_ui_env_phase_switch_by_name(
    plugin_ui_env_t env, const char * phase_name, const char * loading_phase_name,
    dr_data_t data);
    
int plugin_ui_env_phase_reset(plugin_ui_env_t env);
int plugin_ui_env_phase_back(plugin_ui_env_t env);

/*state */
plugin_ui_state_node_t
plugin_ui_env_state_call(
    plugin_ui_env_t env, const char * state_name, const char * loading_state_name, const char * back_state_name,
    plugin_ui_renter_policy_t renter_policy, uint8_t suspend_old, dr_data_t data);
int plugin_ui_env_state_switch(
    plugin_ui_env_t env, const char * state_name, const char * loading_state_name, const char * back_state_name,
    dr_data_t data);
int plugin_ui_env_state_reset(plugin_ui_env_t env);
int plugin_ui_env_state_back(plugin_ui_env_t env);

/*language */
int plugin_ui_env_set_language(plugin_ui_env_t env, const char * str_language);
const char * plugin_ui_env_language(plugin_ui_env_t env);

/*packages*/
int plugin_ui_env_load_package_to_queue_async(
    plugin_ui_env_t env, const char * package_name, const char * queue_name, plugin_package_load_task_t task);

int plugin_ui_env_load_package_async(
    plugin_ui_env_t env, const char * package_name, plugin_ui_package_scope_t scope, plugin_package_load_task_t task);

/*locate*/
int plugin_ui_env_calc_world_pos(ui_vector_2_t result, plugin_ui_env_t env, const char * str);

/*navigation*/
int plugin_ui_env_navigation(plugin_ui_env_t env, const char * path, dr_data_t data);

#ifdef __cplusplus
}
#endif

#endif

