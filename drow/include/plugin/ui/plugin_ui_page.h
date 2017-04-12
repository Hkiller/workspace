#ifndef DROW_PLUGIN_UI_PAGE_H
#define DROW_PLUGIN_UI_PAGE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_it {
    plugin_ui_page_t (*next)(struct plugin_ui_page_it * it);
    char m_data[64];
};
    
plugin_ui_page_t plugin_ui_page_create(
    plugin_ui_env_t env, const char * page_name, plugin_ui_page_meta_t page_meta);
void plugin_ui_page_free(plugin_ui_page_t ui_page);

gd_app_context_t plugin_ui_page_app(plugin_ui_page_t page);
plugin_ui_env_t plugin_ui_page_env(plugin_ui_page_t page);
plugin_ui_page_meta_t plugin_ui_page_meta(plugin_ui_page_t page);

plugin_ui_page_t plugin_ui_page_find(plugin_ui_env_t env, const char * page_name);

ui_data_src_t plugin_ui_page_src(plugin_ui_page_t page);
void plugin_ui_page_set_src(plugin_ui_page_t page, ui_data_src_t src);
int plugin_ui_page_set_src_by_path(plugin_ui_page_t page, const char * load_from);

plugin_ui_page_load_policy_t plugin_ui_page_load_policy(plugin_ui_page_t page);
void plugin_ui_page_set_load_policy(plugin_ui_page_t page, plugin_ui_page_load_policy_t policy);
    
uint8_t plugin_ui_page_is_loaded(plugin_ui_page_t page);
void plugin_ui_page_unload(plugin_ui_page_t page);
int plugin_ui_page_load(plugin_ui_page_t page);

int plugin_ui_page_load_by_path(plugin_ui_page_t page, const char * load_from);

plugin_ui_popup_t plugin_ui_page_visible_in_popup(plugin_ui_page_t page);
uint8_t plugin_ui_page_have_popup(plugin_ui_page_t page);
plugin_ui_popup_t plugin_ui_page_find_popup_by_name(plugin_ui_page_t page, const char * popup_name);

const char * plugin_ui_page_name(plugin_ui_page_t page);

void * plugin_ui_page_product(plugin_ui_page_t page);
plugin_ui_page_t plugin_ui_page_from_product(void * p);
uint16_t plugin_ui_page_product_capacity(plugin_ui_page_t page);

uint8_t plugin_ui_page_force_change(plugin_ui_page_t page);
void plugin_ui_page_set_force_change(plugin_ui_page_t page, uint8_t force_change);
    
uint8_t plugin_ui_page_changed(plugin_ui_page_t page);
void plugin_ui_page_set_changed(plugin_ui_page_t page, uint8_t changed);

uint8_t plugin_ui_page_visible(plugin_ui_page_t page);
uint8_t plugin_ui_page_is_in_render(plugin_ui_page_t page);

uint8_t plugin_ui_page_is_show_in_state(plugin_ui_page_t page, plugin_ui_state_node_t state_node);
    
typedef void (*plugin_ui_page_state_on_hide_fun_t)(void *ctx, plugin_ui_page_t page, plugin_ui_state_node_t state_node);
int plugin_ui_page_show_in_state(
    plugin_ui_page_t page, plugin_ui_state_node_t state_node, dr_data_source_t data_source, const char * before_page,
    plugin_ui_page_state_on_hide_fun_t on_hide_fun, void * on_hide_ctx);

void plugin_ui_page_clear_hide_monitor(plugin_ui_page_t page, void * on_hide_ctx);
    
void plugin_ui_page_hide_in_state(plugin_ui_page_t page, plugin_ui_state_node_t state_node);

plugin_ui_state_node_t plugin_ui_page_top_state_node(plugin_ui_page_t page);

int plugin_ui_page_show_in_current_state(
    plugin_ui_page_t page, dr_data_source_t data_source, const char * before_page,
    plugin_ui_page_state_on_hide_fun_t on_hide_fun, void * on_hide_ctx);
    
void plugin_ui_page_hide(plugin_ui_page_t page);

int plugin_ui_page_sync_state_data(plugin_ui_page_t page, dr_data_source_t data_source);

uint8_t plugin_ui_page_modal(plugin_ui_page_t page);
void plugin_ui_page_set_modal(plugin_ui_page_t page, uint8_t is_modal);
    
plugin_ui_control_t plugin_ui_page_root_control(plugin_ui_page_t ui_page);

uint16_t plugin_ui_page_control_count(plugin_ui_page_t page);

/*package*/
int plugin_ui_page_pacakge_add(plugin_ui_page_t page, const char * package_name);
void plugin_ui_page_pacakge_remove_all(plugin_ui_page_t page);
int plugin_ui_page_pacakge_load_all_async(plugin_ui_page_t page, plugin_package_load_task_t task);
int plugin_ui_page_pacakge_load_all_sync(plugin_ui_page_t page);
    
/*load */
    
/*page data*/
void plugin_ui_page_set_data(plugin_ui_page_t page, LPDRMETA meta, void * data, uint32_t data_size);
LPDRMETA plugin_ui_page_data_meta(plugin_ui_page_t page);
void * plugin_ui_page_data(plugin_ui_page_t page);
uint32_t plugin_ui_page_data_size(plugin_ui_page_t page);
    
/*event */
void plugin_ui_page_send_event(plugin_ui_page_t page, LPDRMETA meta, void * data, size_t data_size);
void plugin_ui_page_build_and_send_event(plugin_ui_page_t page, const char * def, dr_data_source_t data_source);

#define plugin_ui_page_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

