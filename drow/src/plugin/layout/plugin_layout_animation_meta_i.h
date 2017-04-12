#ifndef PLUGIN_LAYOUT_ANIMATION_META_I_H
#define PLUGIN_LAYOUT_ANIMATION_META_I_H
#include "plugin/layout/plugin_layout_animation_meta.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_animation_meta {
    plugin_layout_module_t m_module;
    const char * m_name;
    struct cpe_hash_entry m_hh;
    
    size_t m_anim_capacity;
    void * m_ctx;
    plugin_layout_animation_init_fun_t m_init_fun;
    plugin_layout_animation_free_fun_t m_fini_fun;
    plugin_layout_animation_layout_fun_t m_layout_fun;
    plugin_layout_animation_update_fun_t m_update_fun;
    plugin_layout_animation_render_fun_t m_render_fun;
    
    plugin_layout_animation_list_t m_animations;
};

void plugin_layout_animation_meta_free_all(const plugin_layout_module_t module);
    
uint32_t plugin_layout_animation_meta_hash(const plugin_layout_animation_meta_t meta);
int plugin_layout_animation_meta_eq(const plugin_layout_animation_meta_t l, const plugin_layout_animation_meta_t r);
    

#ifdef __cplusplus
}
#endif

#endif
