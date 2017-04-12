#ifndef PLUGIN_UI_MODULE_I_H
#define PLUGIN_UI_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/xcalc/xcalc_types.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_ui_control_action_slots * plugin_ui_control_action_slots_t;
typedef struct plugin_ui_control_binding_use_slot * plugin_ui_control_binding_use_slot_t;
typedef struct plugin_ui_state_node_page * plugin_ui_state_node_page_t;
typedef struct plugin_ui_state_node_data * plugin_ui_state_node_data_t;
typedef struct plugin_ui_aspect_ref * plugin_ui_aspect_ref_t;

typedef TAILQ_HEAD(plugin_ui_env_list, plugin_ui_env) plugin_ui_env_list_t;
typedef TAILQ_HEAD(plugin_ui_env_action_list, plugin_ui_env_action) plugin_ui_env_action_list_t;
typedef TAILQ_HEAD(plugin_ui_touch_track_list, plugin_ui_touch_track) plugin_ui_touch_track_list_t;
typedef TAILQ_HEAD(plugin_ui_package_queue_managed_list, plugin_ui_package_queue_managed) plugin_ui_package_queue_managed_list_t;
typedef TAILQ_HEAD(plugin_ui_package_queue_using_list, plugin_ui_package_queue_using) plugin_ui_package_queue_using_list_t;
typedef TAILQ_HEAD(plugin_ui_page_list, plugin_ui_page) plugin_ui_page_list_t;
typedef TAILQ_HEAD(plugin_ui_page_eh_list, plugin_ui_page_eh) plugin_ui_page_eh_list_t;
typedef TAILQ_HEAD(plugin_ui_page_plugin_list, plugin_ui_page_plugin) plugin_ui_page_plugin_list_t;    
typedef TAILQ_HEAD(plugin_ui_page_slot_list, plugin_ui_page_slot) plugin_ui_page_slot_list_t;
typedef TAILQ_HEAD(plugin_ui_aspect_list, plugin_ui_aspect) plugin_ui_aspect_list_t;
typedef TAILQ_HEAD(plugin_ui_aspect_ref_list, plugin_ui_aspect_ref) plugin_ui_aspect_ref_list_t;
typedef TAILQ_HEAD(plugin_ui_control_list, plugin_ui_control) plugin_ui_control_list_t;
typedef TAILQ_HEAD(plugin_ui_control_category_list, plugin_ui_control_category) plugin_ui_control_category_list_t;
typedef TAILQ_HEAD(plugin_ui_control_action_list, plugin_ui_control_action) plugin_ui_control_action_list_t;
typedef TAILQ_HEAD(plugin_ui_control_timer_list, plugin_ui_control_timer) plugin_ui_control_timer_list_t;
typedef TAILQ_HEAD(plugin_ui_control_binding_list, plugin_ui_control_binding) plugin_ui_control_binding_list_t;
typedef TAILQ_HEAD(plugin_ui_control_binding_use_slot_list, plugin_ui_control_binding_use_slot) plugin_ui_control_binding_use_slot_list_t;
typedef TAILQ_HEAD(plugin_ui_control_frame_list, plugin_ui_control_frame) plugin_ui_control_frame_list_t;
typedef TAILQ_HEAD(plugin_ui_phase_list, plugin_ui_phase) plugin_ui_phase_list_t;
typedef TAILQ_HEAD(plugin_ui_phase_use_page_list, plugin_ui_phase_use_page) plugin_ui_phase_use_page_list_t;
typedef TAILQ_HEAD(plugin_ui_phase_use_popup_def_list, plugin_ui_phase_use_popup_def) plugin_ui_phase_use_popup_def_list_t;
typedef TAILQ_HEAD(plugin_ui_phase_node_list, plugin_ui_phase_node) plugin_ui_phase_node_list_t;
typedef TAILQ_HEAD(plugin_ui_state_node_list, plugin_ui_state_node) plugin_ui_state_node_list_t;
typedef TAILQ_HEAD(plugin_ui_navigation_list, plugin_ui_navigation) plugin_ui_navigation_list_t;
typedef TAILQ_HEAD(plugin_ui_state_node_page_list, plugin_ui_state_node_page) plugin_ui_state_node_page_list_t;
typedef TAILQ_HEAD(plugin_ui_popup_def_list, plugin_ui_popup_def) plugin_ui_popup_def_list_t;
typedef TAILQ_HEAD(plugin_ui_popup_def_binding_list, plugin_ui_popup_def_binding) plugin_ui_popup_def_binding_list_t;
typedef TAILQ_HEAD(plugin_ui_popup_def_binding_attr_list, plugin_ui_popup_def_binding_attr) plugin_ui_popup_def_binding_attr_list_t;
typedef TAILQ_HEAD(plugin_ui_popup_list, plugin_ui_popup) plugin_ui_popup_list_t;
typedef TAILQ_HEAD(plugin_ui_popup_action_list, plugin_ui_popup_action) plugin_ui_popup_action_list_t;
typedef TAILQ_HEAD(plugin_ui_exec_step_list, plugin_ui_exec_step) plugin_ui_exec_step_list_t;
typedef TAILQ_HEAD(plugin_ui_animation_list, plugin_ui_animation) plugin_ui_animation_list_t;
typedef TAILQ_HEAD(plugin_ui_animation_control_list, plugin_ui_animation_control) plugin_ui_animation_control_list_t;
typedef TAILQ_HEAD(plugin_ui_move_algorithm_list, plugin_ui_move_algorithm) plugin_ui_move_algorithm_list_t;
    
struct plugin_ui_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_data_mgr_t m_data_mgr;
    plugin_package_module_t m_package_module;
    ui_runtime_module_t m_runtime;
    plugin_editor_module_t m_editor_module;
    uint8_t m_debug;
    float m_cfg_fps;
    xcomputer_t m_computer;

    /*env*/
    plugin_ui_env_list_t m_envs;

    /*Page */
    struct cpe_hash_table m_page_metas;

    /*Animation */
    uint32_t m_animation_max_capacity;
    uint32_t m_animation_control_max_capacity;
    struct cpe_hash_table m_animation_metas;
    
    /*Move algorithm*/
    uint32_t m_move_algorithm_max_capacity;
    struct cpe_hash_table m_move_algorithm_metas;
    
    /*Control */
    uint32_t m_control_max_capacity;
    plugin_ui_control_meta_t m_control_metas;

    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif 
