#ifndef PLUGIN_LAYOUT_LAYOUT_I_H
#define PLUGIN_LAYOUT_LAYOUT_I_H
#include "plugin/layout/plugin_layout_layout.h"
#include "plugin_layout_layout_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_layout {
    plugin_layout_render_t m_render;
    plugin_layout_layout_meta_t m_meta;
    TAILQ_ENTRY(plugin_layout_layout) m_next_for_module;
};

void plugin_layout_layout_real_free(plugin_layout_layout_t layout);
    
#ifdef __cplusplus
}
#endif

#endif
