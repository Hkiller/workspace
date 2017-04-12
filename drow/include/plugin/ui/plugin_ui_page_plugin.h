#ifndef DROW_PLUGIN_UI_PAGE_PLUGIN_H
#define DROW_PLUGIN_UI_PAGE_PLUGIN_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_plugin_it {
    plugin_ui_page_plugin_t (*next)(struct plugin_ui_page_plugin_it * it);
    char m_data[64];
};

typedef int (*plugin_ui_page_plugin_on_init_t)(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin);
typedef void (*plugin_ui_page_plugin_on_fini_t)(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin);
typedef int (*plugin_ui_page_plugin_on_load_t)(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin);
typedef void (*plugin_ui_page_plugin_on_unload_t)(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin);
typedef int (*plugin_ui_page_plugin_on_visiable_t)(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin);
typedef void (*plugin_ui_page_plugin_on_hide_t)(void * ctx, plugin_ui_page_t page, plugin_ui_page_plugin_t plugin);
    
plugin_ui_page_plugin_t
plugin_ui_page_plugin_create(
    plugin_ui_page_t page, void * ctx, size_t capacity,
    plugin_ui_page_plugin_on_init_t on_init,
    plugin_ui_page_plugin_on_fini_t on_fini,
    plugin_ui_page_plugin_on_load_t on_load,
    plugin_ui_page_plugin_on_unload_t on_unload,
    plugin_ui_page_plugin_on_visiable_t on_visiable,
    plugin_ui_page_plugin_on_hide_t on_hide);

void plugin_ui_page_plugin_free(plugin_ui_page_plugin_t plugin);

plugin_ui_page_t plugin_ui_page_plugin_page(plugin_ui_page_plugin_t plugin);
void * plugin_ui_page_plugin_data(plugin_ui_page_plugin_t plugin);
size_t plugin_ui_page_plugin_capacity(plugin_ui_page_plugin_t plugin);

void plugin_ui_page_plugin_clear_by_ctx_in_page(plugin_ui_page_t page, void * ctx);
void plugin_ui_page_plugin_clear_by_ctx_in_env(plugin_ui_env_t env, void * ctx);    
    
#define plugin_ui_page_plugin_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

