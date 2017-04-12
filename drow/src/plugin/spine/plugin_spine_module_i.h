#ifndef PLUGIN_SPINE_MODULE_I_H
#define PLUGIN_SPINE_MODULE_I_H
#include "spine/Skeleton.h"
#include "spine/Animation.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "plugin/spine/plugin_spine_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_spine_track_listener * plugin_spine_track_listener_t;
typedef struct plugin_spine_obj_anim_group_binding * plugin_spine_obj_anim_group_binding_t;
    
typedef TAILQ_HEAD(plugin_spine_obj_track_list, plugin_spine_obj_track) plugin_spine_obj_track_list_t;
typedef TAILQ_HEAD(plugin_spine_obj_anim_list, plugin_spine_obj_anim) plugin_spine_obj_anim_list_t;
typedef TAILQ_HEAD(plugin_spine_obj_part_list, plugin_spine_obj_part) plugin_spine_obj_part_list_t;
typedef TAILQ_HEAD(plugin_spine_obj_part_state_list, plugin_spine_obj_part_state) plugin_spine_obj_part_state_list_t;
typedef TAILQ_HEAD(plugin_spine_obj_part_transition_list, plugin_spine_obj_part_transition) plugin_spine_obj_part_transition_list_t;
typedef TAILQ_HEAD(plugin_spine_obj_anim_group_binding_list, plugin_spine_obj_anim_group_binding) plugin_spine_obj_anim_group_binding_list_t;
typedef TAILQ_HEAD(plugin_spine_obj_ik_list, plugin_spine_obj_ik) plugin_spine_obj_ik_list_t;
    
struct plugin_spine_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;

    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;

    LPDRMETA m_meta_data_part;
    LPDRMETA m_meta_data_part_state;
    LPDRMETA m_meta_data_part_transition;

    plugin_spine_track_listener_t m_free_listeners;
    plugin_spine_obj_part_list_t m_free_parts;
    plugin_spine_obj_part_state_list_t m_free_part_states;
    plugin_spine_obj_part_transition_list_t m_free_part_transitions;
    plugin_spine_obj_track_list_t m_free_tracks;
    plugin_spine_obj_anim_list_t m_free_anims;
    plugin_spine_obj_anim_group_binding_list_t m_free_anim_group_bindings;
    plugin_spine_obj_ik_list_t m_free_iks;
    
    struct mem_buffer m_dump_buffer;

    spEvent * m_events_buf[100];
};

int plugin_spine_module_create_shaders(plugin_spine_module_t module);
void plugin_spine_module_free_shaders(plugin_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
