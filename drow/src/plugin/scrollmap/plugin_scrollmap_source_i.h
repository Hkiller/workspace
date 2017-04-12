#ifndef PLUGIN_SCROLLMAP_DATA_SOURCE_I_H
#define PLUGIN_SCROLLMAP_DATA_SOURCE_I_H
#include "plugin/scrollmap/plugin_scrollmap_source.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_data_scene_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_source {
    plugin_scrollmap_env_t m_env;
    TAILQ_ENTRY(plugin_scrollmap_source) m_next_for_env;
    plugin_scrollmap_data_scene_t m_data;
    plugin_scrollmap_range_list_t m_ranges;
};

#ifdef __cplusplus
}
#endif

#endif
