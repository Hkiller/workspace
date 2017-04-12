#ifndef PLUGIN_UI_ANIMATION_META_I_H
#define PLUGIN_UI_ANIMATION_META_I_H
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_animation_meta {
    plugin_ui_module_t m_module;
    const char * m_name;
    struct cpe_hash_entry m_hh;
    
    size_t m_anim_capacity;
    size_t m_control_capacity;
    void * m_ctx;
    plugin_ui_animation_init_fun_t m_init_fun;
    plugin_ui_animation_free_fun_t m_fini_fun;
    plugin_ui_animation_enter_fun_t m_enter_fun;
    plugin_ui_animation_exit_fun_t m_exit_fun;
    plugin_ui_animation_update_fun_t m_update_fun;
    plugin_ui_animation_control_attach_fun_t m_control_attach;
    plugin_ui_animation_control_detach_fun_t m_control_detach;
    plugin_ui_animation_setup_fun_t m_setup_fun;

    plugin_ui_animation_list_t m_animations;
};

void plugin_ui_animation_meta_free_all(const plugin_ui_module_t module);
    
uint32_t plugin_ui_animation_meta_hash(const plugin_ui_animation_meta_t meta);
int plugin_ui_animation_meta_eq(const plugin_ui_animation_meta_t l, const plugin_ui_animation_meta_t r);
    

#ifdef __cplusplus
}
#endif

#endif
