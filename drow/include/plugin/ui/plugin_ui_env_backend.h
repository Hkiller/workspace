#ifndef DROW_PLUGIN_UI_ENV_BACKEND_H
#define DROW_PLUGIN_UI_ENV_BACKEND_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_env_backend {
    void * ctx;
    void (*send_event)(void * ctx, plugin_ui_env_t env, LPDRMETA meta, void * data, uint32_t data_size, dr_data_t overwrite);
    void (*build_and_send_event)(void * ctx, plugin_ui_env_t env, const char * def, dr_data_source_t data_source, dr_data_t overwrite);
    uint32_t popup_def_capacity;
    int (*popup_def_init)(void * ctx, plugin_ui_env_t env, plugin_ui_popup_def_t popup_def);
    void (*popup_def_fini)(void * ctx, plugin_ui_env_t env, plugin_ui_popup_def_t popup_def);
    int (*popup_enter)(void * ctx, plugin_ui_env_t env, plugin_ui_popup_t popup);
    void (*popup_leave)(void * ctx, plugin_ui_env_t env, plugin_ui_popup_t popup);
    uint32_t phase_capacity;
    int (*phase_init)(void * ctx, plugin_ui_env_t env, plugin_ui_phase_t phase);
    void (*phase_fini)(void * ctx, plugin_ui_env_t env, plugin_ui_phase_t phase);
    int (*phase_enter)(void * ctx, plugin_ui_env_t env, plugin_ui_phase_t phase);
    void (*phase_leave)(void * ctx, plugin_ui_env_t env, plugin_ui_phase_t phase);
    uint32_t state_capacity;
    int (*state_init)(void * ctx, plugin_ui_state_t state);
    void (*state_fini)(void * ctx, plugin_ui_state_t state);
    int (*state_node_active)(void * ctx, plugin_ui_state_node_t state_node, plugin_ui_state_t state);
    uint8_t (*state_node_is_active)(void * ctx, plugin_ui_state_node_t state_node);
    void (*state_node_deactive)(void * ctx, plugin_ui_state_node_t state_node);
    uint32_t navigation_capacity;
    int (*navigation_init)(void * ctx, plugin_ui_navigation_t navigation);
    void (*navigation_fini)(void * ctx, plugin_ui_navigation_t navigation);
    uint32_t eh_capacity;
    int (*eh_init)(void * ctx, plugin_ui_page_eh_t eh);
    void (*eh_fini)(void * ctx, plugin_ui_page_eh_t eh);
    int (*eh_active)(void * ctx, plugin_ui_page_eh_t eh);
    void (*eh_deactive)(void * ctx, plugin_ui_page_eh_t eh);
};
    
#ifdef __cplusplus
}
#endif

#endif

