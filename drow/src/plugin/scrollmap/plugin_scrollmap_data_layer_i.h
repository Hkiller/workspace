#ifndef PLUGIN_SCROLLMAP_DATA_LAYER_I_H
#define PLUGIN_SCROLLMAP_DATA_LAYER_I_H
#include "plugin_scrollmap_data_scene_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_data_layer {
    plugin_scrollmap_data_scene_t m_scene;
    TAILQ_ENTRY(plugin_scrollmap_data_layer) m_next;
    SCROLLMAP_LAYER m_data;
    uint32_t m_block_count;
    plugin_scrollmap_data_block_list_t m_blocks;
    uint32_t m_script_count;
    plugin_scrollmap_data_script_list_t m_scripts;
};

void plugin_scrollmap_data_layer_real_free(plugin_scrollmap_data_layer_t layer);
    
#ifdef __cplusplus
}
#endif

#endif
