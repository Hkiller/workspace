#ifndef PLUGIN_SCROLLMAP_SOURCE_H
#define PLUGIN_SCROLLMAP_SOURCE_H
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_source_it {
    plugin_scrollmap_source_t (*next)(plugin_scrollmap_source_it_t it);
    char m_data[64];
};

plugin_scrollmap_source_t plugin_scrollmap_source_create(plugin_scrollmap_env_t env, ui_data_src_t src);
plugin_scrollmap_source_t plugin_scrollmap_source_create_by_path(plugin_scrollmap_env_t env, const char * path);
plugin_scrollmap_source_t plugin_scrollmap_source_find(plugin_scrollmap_env_t env, ui_data_src_t src);
plugin_scrollmap_source_t plugin_scrollmap_source_find_by_path(plugin_scrollmap_env_t env, const char * path);
void plugin_scrollmap_source_free(plugin_scrollmap_source_t source);

ui_data_src_t plugin_scrollmap_source_src(plugin_scrollmap_source_t source);
plugin_scrollmap_data_scene_t plugin_scrollmap_source_data(plugin_scrollmap_source_t source);
void plugin_scrollmap_env_sources(plugin_scrollmap_source_it_t it, plugin_scrollmap_env_t env);
    
#define plugin_scrollmap_source_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
