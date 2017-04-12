#ifndef PLUGIN_LAYOUT_ANIMATION_I_H
#define PLUGIN_LAYOUT_ANIMATION_I_H
#include "plugin/layout/plugin_layout_animation.h"
#include "plugin_layout_module_i.h"
#include "plugin_layout_animation_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct plugin_layout_animation {
    plugin_layout_render_t m_render;
    TAILQ_ENTRY(plugin_layout_animation) m_next_for_render;
    plugin_layout_animation_meta_t m_meta;
    TAILQ_ENTRY(plugin_layout_animation) m_next_for_meta;
    struct cpe_hash_entry m_hh;
    uint32_t m_id;
};

void plugin_layout_animation_real_free(plugin_layout_animation_t animation);

uint32_t plugin_layout_animation_hash(const plugin_layout_animation_t meta);
int plugin_layout_animation_eq(const plugin_layout_animation_t l, const plugin_layout_animation_t r);
    
#ifdef __cplusplus
}
#endif

#endif
