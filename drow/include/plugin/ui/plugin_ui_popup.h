#ifndef DROW_PLUGIN_UI_POPUP_H
#define DROW_PLUGIN_UI_POPUP_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_it {
    plugin_ui_popup_t (*next)(struct plugin_ui_popup_it * it);
    char m_data[64];
};

plugin_ui_popup_t plugin_ui_popup_create(plugin_ui_env_t env, const char * name, plugin_ui_page_meta_t page_meta, LPDRMETA data_meta);
void plugin_ui_popup_free(plugin_ui_popup_t ui_popup);

plugin_ui_popup_t plugin_ui_popup_find_first_by_name(plugin_ui_env_t env, const char * name);
plugin_ui_popup_t plugin_ui_popup_find_by_id(plugin_ui_env_t env, uint32_t popup_id);

plugin_ui_popup_t plugin_ui_popup_first(plugin_ui_env_t env);
plugin_ui_popup_t plugin_ui_popup_prev(plugin_ui_popup_t popup);
plugin_ui_popup_t plugin_ui_popup_next(plugin_ui_popup_t popup);

uint32_t plugin_ui_popup_id(plugin_ui_popup_t popup);

plugin_ui_env_t plugin_ui_popup_env(plugin_ui_popup_t popup);
plugin_ui_page_t plugin_ui_popup_page(plugin_ui_popup_t popup);

const char * plugin_ui_popup_name(plugin_ui_popup_t popup);
plugin_ui_popup_def_t plugin_ui_popup_def(plugin_ui_popup_t popup);
    
int16_t plugin_ui_popup_layer(plugin_ui_popup_t popup);
void plugin_ui_popup_set_layer(plugin_ui_popup_t popup, int16_t layer);

plugin_ui_page_t plugin_ui_popup_create_from_page(plugin_ui_popup_t popup);
void plugin_ui_popup_set_create_from_page(plugin_ui_popup_t popup, plugin_ui_page_t page);

void plugin_ui_popup_close_by_data_meta(plugin_ui_env_t env, LPDRMETA data_meta);    

void plugin_ui_popup_close_from_page(plugin_ui_page_t page);
void plugin_ui_popup_close_from_page_by_data_meta(plugin_ui_page_t page, LPDRMETA data_meta);    
void plugin_ui_popup_close_from_page_by_name(plugin_ui_page_t page, const char * popup_name);
    
float plugin_ui_popup_lifecircle(plugin_ui_popup_t popup);
void plugin_ui_popup_set_lifecircle(plugin_ui_popup_t popup, float lifecircle);
    
int plugin_ui_popup_set_data(plugin_ui_popup_t popup, dr_data_t data);

uint8_t plugin_ui_popup_visible(plugin_ui_popup_t popup);
int plugin_ui_popup_set_visible(plugin_ui_popup_t popup, uint8_t visible);

int plugin_ui_popup_set_close_on_click(plugin_ui_popup_t popup, plugin_ui_control_t c);
int plugin_ui_popup_set_action_on_click(plugin_ui_popup_t popup, plugin_ui_control_t c, const char * action);
plugin_ui_control_t plugin_ui_popup_find_action_control(plugin_ui_popup_t popup, const char * action);
uint16_t plugin_ui_popup_trigger_action(plugin_ui_popup_t popup, const char * action_name);

#define plugin_ui_popup_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

