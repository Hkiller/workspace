#ifndef PLUGIN_SCROLLMAP_SCRIPT_I_H
#define PLUGIN_SCROLLMAP_SCRIPT_I_H
#include "plugin/scrollmap/plugin_scrollmap_script.h"
#include "plugin_scrollmap_layer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_script {
    plugin_scrollmap_layer_t m_layer;
    TAILQ_ENTRY(plugin_scrollmap_script) m_next_for_layer;
    plugin_scrollmap_range_t m_range;
    TAILQ_ENTRY(plugin_scrollmap_script) m_next_for_range;
    plugin_scrollmap_script_state_t m_state;
    ui_vector_2 m_pos;
    SCROLLMAP_SCRIPT m_script;
};

void plugin_scrollmap_script_real_free(plugin_scrollmap_script_t script);

void plugin_scrollmap_script_update(plugin_scrollmap_env_t env, plugin_scrollmap_script_t script, float delta_len, float delta_s);
    
#ifdef __cplusplus
}
#endif

#endif
