#ifndef PLUGIN_SCROLLMAP_DATA_SCRIPT_I_H
#define PLUGIN_SCROLLMAP_DATA_SCRIPT_I_H
#include "plugin_scrollmap_data_layer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_data_script {
    plugin_scrollmap_data_layer_t m_layer;
    TAILQ_ENTRY(plugin_scrollmap_data_script) m_next;
    SCROLLMAP_SCRIPT m_data;
};

void plugin_scrollmap_data_script_real_free(plugin_scrollmap_data_script_t script);
    
#ifdef __cplusplus
}
#endif

#endif
