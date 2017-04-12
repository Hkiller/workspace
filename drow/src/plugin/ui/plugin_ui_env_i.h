#ifndef PLUGIN_UI_ENV_I_H
#define PLUGIN_UI_ENV_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/dr/dr_types.h"
#include "render/utils/ui_vector_2.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_cfg.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_env {
    plugin_ui_module_t m_module;
    TAILQ_ENTRY(plugin_ui_env) m_next_for_module;

    uint8_t m_debug;
    ui_string_table_t m_strings;
    plugin_ui_env_backend_t m_backend;
    uint8_t m_anim_base_fps;
    cfg_t m_cfg;
    
    plugin_ui_env_action_list_t m_env_actions;

    uint32_t m_tick;
    uint32_t m_visible_page_count;
    uint32_t m_page_count;
    uint32_t m_control_count;
    uint32_t m_control_free_count;    

    ui_runtime_render_camera_t m_camera;
    
    uint8_t m_visible_pages_need_update;
    plugin_ui_page_list_t m_visible_pages;
    plugin_ui_page_list_t m_hiding_pages;    
    plugin_ui_page_list_t m_pages;
    plugin_ui_page_plugin_list_t m_page_plugins;
    struct cpe_hash_table m_pages_by_name;
    plugin_ui_page_t m_template_page;

    uint8_t m_package_need_gc;
    
    /*全局锁定资源 */
    plugin_package_group_t m_lock_packages;
    
    /*下一个阶段需要的附加资源 */
    plugin_package_group_t m_next_phase_loading_packages;
    plugin_package_group_t m_next_phase_runing_packages;
    plugin_package_group_t m_next_state_loading_packages;
    plugin_package_group_t m_next_state_runing_packages;
    
    /*package mamaged queue*/
    plugin_ui_package_queue_managed_list_t m_package_queue_manageds;
    
    /*control category*/
    plugin_ui_control_category_list_t m_control_categories;

    /*aspects*/
    plugin_ui_aspect_t m_lock_aspect;
    plugin_ui_aspect_list_t m_aspects;

    /*phase*/
    plugin_ui_phase_t m_init_phase;
    plugin_ui_phase_t m_init_call_phase;
    uint8_t m_init_call_phase_auto_complete;
    plugin_ui_phase_node_list_t m_phase_stack;
    plugin_ui_phase_list_t m_phases;

    /*double click*/
    plugin_ui_control_t m_double_click_control;
    float m_double_click_duration;
    float m_double_click_span;
    
    /*config*/
    ui_vector_2 m_runtime_sz;
    ui_vector_2 m_origin_sz;
    plugin_ui_screen_resize_policy_t m_screen_resize_policy;
    ui_vector_2 m_screen_adj;

    /*animation*/
    uint32_t m_max_animation_id;
    struct cpe_hash_table m_animations;
    float m_animation_wait_runging_time;
    plugin_ui_animation_list_t m_init_animations;
    plugin_ui_animation_list_t m_wait_animations;
    plugin_ui_animation_list_t m_working_animations;
    plugin_ui_animation_list_t m_done_animations;
    plugin_ui_animation_list_t m_keep_animations;
    plugin_ui_animation_list_t m_free_animations;
    plugin_ui_animation_control_list_t m_free_animation_controls;

    /*move algorithm*/
    uint32_t m_max_move_algorithm_id;
    struct cpe_hash_table m_move_algorithms;
    plugin_ui_move_algorithm_list_t m_free_move_algorithms;
    
    /*focuse*/
    plugin_ui_control_t m_focus_control;

    /*float*/
    plugin_ui_control_t m_float_control;
    
    /*popups*/
    uint32_t m_popup_max_id;
    plugin_ui_popup_def_list_t m_popup_defs;
    plugin_ui_popup_list_t m_popups;

    /*buff*/
    plugin_ui_env_action_list_t m_free_env_actions;
    plugin_ui_aspect_list_t m_free_aspects;
    plugin_ui_aspect_ref_list_t m_free_aspect_refs;
    plugin_ui_control_list_t m_free_controls;
    plugin_ui_control_action_slots_t m_free_control_action_slots;
    plugin_ui_control_action_list_t m_free_control_actions;
    plugin_ui_control_frame_list_t m_free_frames;
    plugin_ui_control_timer_list_t m_free_timers;
    plugin_ui_control_binding_list_t m_free_bindings;
    plugin_ui_control_binding_use_slot_list_t m_free_binding_use_slots;
    plugin_ui_phase_node_list_t m_free_phase_nodes;
    plugin_ui_state_node_list_t m_free_state_nodes;
    plugin_ui_page_eh_list_t m_free_page_ehs;
    plugin_ui_page_slot_list_t m_free_page_slots;
    plugin_ui_state_node_page_list_t m_free_node_pages;
    plugin_ui_popup_list_t m_free_popups;
    plugin_ui_popup_action_list_t m_free_popup_actions;

    /*mouse*/
    plugin_ui_mouse_t m_mouse;
    
    /*touch*/
    uint8_t m_accept_input;
    uint8_t m_accept_multi_touch;
    float m_long_push_span;
    plugin_ui_touch_track_list_t m_touch_tracks;
    plugin_ui_touch_track_list_t m_free_touch_tracks;

    /*cfg*/
    struct ui_runtime_render_second_color m_cfg_down_color;
    struct plugin_ui_cfg_button m_cfg_button;
    struct plugin_ui_cfg_button m_cfg_toggle;
};

void plugin_ui_env_set_double_click_control(plugin_ui_env_t env, plugin_ui_control_t ctrl);    
void plugin_ui_control_evt_on_control_remove(plugin_ui_env_t env, plugin_ui_control_t control);

int plugin_ui_env_init_lock_packages(plugin_ui_env_t env);
    
void plugin_ui_env_update_phase(plugin_ui_env_t env);
void plugin_ui_env_update_state(plugin_ui_phase_node_t phase_node, uint8_t can_restart);
void plugin_ui_env_update_visible_pages(plugin_ui_env_t env);
void plugin_ui_env_update_popups(plugin_ui_env_t env, float delta);
void plugin_ui_env_update_hiding_pages(plugin_ui_env_t env);
void plugin_ui_env_update_double_click(plugin_ui_env_t env, float delta);

void plugin_ui_env_dispatch_event(plugin_ui_env_t env, plugin_ui_control_t from, plugin_ui_event_t evt);

void plugin_ui_env_cfg_init(plugin_ui_env_t env);

void plugin_ui_env_package_gc(plugin_ui_env_t env);
    
#ifdef __cplusplus
}
#endif

#endif
