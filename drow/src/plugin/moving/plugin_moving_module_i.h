#ifndef PLUGIN_MOVING_MODULE_I_H
#define PLUGIN_MOVING_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/moving/plugin_moving_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_moving_plan_point_list, plugin_moving_plan_point) plugin_moving_plan_point_list_t;
typedef TAILQ_HEAD(plugin_moving_plan_track_list, plugin_moving_plan_track) plugin_moving_plan_track_list_t;
typedef TAILQ_HEAD(plugin_moving_plan_node_list, plugin_moving_plan_node) plugin_moving_plan_node_list_t;
typedef TAILQ_HEAD(plugin_moving_plan_segment_list, plugin_moving_plan_segment) plugin_moving_plan_segment_list_t;
    
struct plugin_moving_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_data_mgr_t m_data_mgr;
    uint8_t m_debug;

    LPDRMETA m_meta_moving_plan;
    LPDRMETA m_meta_moving_plan_track;
    LPDRMETA m_meta_moving_plan_point;
    LPDRMETA m_meta_moving_plan_node;
    LPDRMETA m_meta_moving_plan_segment;
    
    plugin_moving_plan_point_list_t m_free_plan_points;
    plugin_moving_plan_node_list_t m_free_plan_nodes;
    plugin_moving_plan_track_list_t m_free_plan_tracks;
    plugin_moving_plan_segment_list_t m_free_plan_segments;
    
    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif 
