#ifndef DROW_PLUGIN_UI_PAGE_META_H
#define DROW_PLUGIN_UI_PAGE_META_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_ui_page_update_fun_t)(plugin_ui_page_t page, float delta);    
typedef void (*plugin_ui_page_event_fun_t)(plugin_ui_page_t page);
    
plugin_ui_page_meta_t
plugin_ui_page_meta_create(
    plugin_ui_module_t module,
    const char * page_type_name,
    uint32_t data_capacity,
    plugin_ui_page_init_fun_t init_fun,
    plugin_ui_page_fini_fun_t fini_fun);

plugin_ui_page_meta_t
plugin_ui_page_meta_load_from_module(plugin_ui_module_t module, const char * type_name);
    
void plugin_ui_page_meta_free(plugin_ui_page_meta_t meta);

plugin_ui_page_meta_t
plugin_ui_page_meta_find(plugin_ui_module_t module, const char * name);

void plugin_ui_page_meta_set_on_update(plugin_ui_page_meta_t meta, plugin_ui_page_update_fun_t on_update);
void plugin_ui_page_meta_set_on_changed(plugin_ui_page_meta_t meta, plugin_ui_page_event_fun_t on_changed);
void plugin_ui_page_meta_set_on_hide(plugin_ui_page_meta_t meta, plugin_ui_page_event_fun_t on_hide);
void plugin_ui_page_meta_set_on_load(plugin_ui_page_meta_t meta, plugin_ui_page_event_fun_t on_load);
    
#ifdef __cplusplus
}
#endif

#endif

