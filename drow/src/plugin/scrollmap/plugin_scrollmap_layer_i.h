#ifndef PLUGIN_SCROLLMAP_LAYER_I_H
#define PLUGIN_SCROLLMAP_LAYER_I_H
#include "plugin/scrollmap/plugin_scrollmap_layer.h"
#include "plugin_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_layer {
    plugin_scrollmap_env_t m_env;
    TAILQ_ENTRY(plugin_scrollmap_layer) m_next_for_env;
    char m_name[64];
    float m_curent_pos;
    plugin_scrollmap_range_list_t m_idle_ranges;
    plugin_scrollmap_range_list_t m_active_ranges;
    plugin_scrollmap_range_list_t m_canceling_ranges;
    plugin_scrollmap_range_list_t m_done_ranges;
    plugin_scrollmap_block_list_t m_blocks;
    plugin_scrollmap_script_list_t m_scripts;
    plugin_scrollmap_obj_list_t m_land_objs;
    float m_speed_adj;
};

void plugin_scrollmap_layer_update(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, float delta_len, float delta_s);

#ifdef __cplusplus
}
#endif

#endif
