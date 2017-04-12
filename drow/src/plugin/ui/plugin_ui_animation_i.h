#ifndef PLUGIN_UI_ANIMATION_I_H
#define PLUGIN_UI_ANIMATION_I_H
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_animation_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct plugin_ui_animation {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_animation) m_next_for_env;
    plugin_ui_animation_meta_t m_meta;
    TAILQ_ENTRY(plugin_ui_animation) m_next_for_meta;
    char * m_name;
    struct cpe_hash_entry m_hh;
    uint32_t m_id;
    uint8_t m_auto_free;
    uint8_t m_is_processing;
    float m_delay;
    uint32_t m_loop_count;
    float m_loop_delay;
    plugin_ui_animation_state_t m_state;
    plugin_ui_animation_control_list_t m_controls;
    plugin_ui_aspect_ref_list_t m_aspects;
    plugin_ui_aspect_t m_aspect;

    struct {
        void * m_ctx;
        plugin_ui_animation_fun_t m_fun;
        void * m_arg;
        void (*m_arg_free)(void *);
    } m_on_complete;
};

void plugin_ui_animation_free_all(const plugin_ui_env_t env);    
void plugin_ui_animation_real_free(plugin_ui_animation_t animation);

void plugin_ui_animation_set_state(plugin_ui_animation_t animation, plugin_ui_animation_state_t state);

void plugin_ui_env_update_animations(plugin_ui_env_t env, float delta);

void plugin_ui_animation_check_notify_complete(plugin_ui_animation_t animation);

uint32_t plugin_ui_animation_hash(const plugin_ui_animation_t meta);
int plugin_ui_animation_eq(const plugin_ui_animation_t l, const plugin_ui_animation_t r);
    
#ifdef __cplusplus
}
#endif

#endif
