#ifndef DROW_PLUGIN_UI_POPUP_DEF_H
#define DROW_PLUGIN_UI_POPUP_DEF_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_def_it {
    plugin_ui_popup_def_t (*next)(struct plugin_ui_popup_def_it * it);
    char m_data[64];
};
    
plugin_ui_popup_def_t plugin_ui_popup_def_create(plugin_ui_env_t env, const char * popup_def_name);
void plugin_ui_popup_def_free(plugin_ui_popup_def_t ui_popup_def);

plugin_ui_popup_def_t plugin_ui_popup_def_find(plugin_ui_env_t env, const char * popup_def_name);

const char * plugin_ui_popup_def_name(plugin_ui_popup_def_t popup_def);
void * plugin_ui_popup_def_data(plugin_ui_popup_def_t popup_def);

int16_t plugin_ui_popup_def_layer(plugin_ui_popup_def_t popup_def);
void plugin_ui_popup_def_set_layer(plugin_ui_popup_def_t popup_def, int16_t layer);

float plugin_ui_popup_def_lifecircle(plugin_ui_popup_def_t popup_def);
void plugin_ui_popup_def_set_lifecircle(plugin_ui_popup_def_t popup_def, float lifecircle);

plugin_ui_page_meta_t plugin_ui_popup_def_page_meta(plugin_ui_popup_def_t popup_def);
void plugin_ui_popup_def_set_page_meta(plugin_ui_popup_def_t popup_def, plugin_ui_page_meta_t page_meta);

LPDRMETA plugin_ui_popup_def_data_meta(plugin_ui_popup_def_t popup_def);
void plugin_ui_popup_def_set_data_meta(plugin_ui_popup_def_t popup_def, LPDRMETA data_meta);
    
const char * plugin_ui_popup_def_page_load_from(plugin_ui_popup_def_t popup_def);
int plugin_ui_popup_def_set_page_load_from_by_path(plugin_ui_popup_def_t popup_def, const char * path);
    
plugin_ui_popup_t plugin_ui_popup_def_create_popup(plugin_ui_popup_def_t popup_def);
    
#define plugin_ui_popup_def_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

