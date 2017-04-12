#ifndef PLUGIN_SCROLLMAP_DATA_BLOCK_I_H
#define PLUGIN_SCROLLMAP_DATA_BLOCK_I_H
#include "plugin_scrollmap_data_layer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_data_block {
    plugin_scrollmap_data_layer_t m_layer;
    TAILQ_ENTRY(plugin_scrollmap_data_block) m_next;
    SCROLLMAP_BLOCK m_data;
};

void plugin_scrollmap_data_block_real_free(plugin_scrollmap_data_block_t block);
    
#ifdef __cplusplus
}
#endif

#endif
