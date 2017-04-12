#ifndef PLUGIN_MOVING_PLAN_I_H
#define PLUGIN_MOVING_PLAN_I_H
#include "cpe/pal/pal_queue.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/moving/plugin_moving_plan.h"
#include "plugin_moving_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan {
    plugin_moving_module_t m_module;
    ui_data_src_t m_src;
    plugin_moving_plan_track_list_t m_tracks;
    plugin_moving_plan_node_list_t m_nodes;
    uint16_t m_track_count;
    uint16_t m_node_count;
    MOVING_PLAN m_data;
};

#ifdef __cplusplus
}
#endif

#endif
