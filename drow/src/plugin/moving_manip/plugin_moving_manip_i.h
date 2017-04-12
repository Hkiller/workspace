#ifndef PLUGIN_MOVING_MANIP_I_H
#define PLUGIN_MOVING_MANIP_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/moving_manip/plugin_moving_manip.h"
#include "plugin/moving/plugin_moving_plan.h"
#include "plugin/moving/plugin_moving_plan_track.h"
#include "plugin/moving/plugin_moving_plan_point.h"
#include "plugin/moving/plugin_moving_plan_node.h"
#include "plugin/moving/plugin_moving_plan_segment.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_ed_mgr_t m_ed_mgr;
    plugin_moving_module_t m_moving_module;
    uint8_t m_debug;
};

int plugin_moving_ed_src_load(ui_ed_src_t src);

ui_ed_obj_t plugin_moving_plan_track_ed_obj_create(ui_ed_obj_t parent);
ui_ed_obj_t plugin_moving_plan_point_ed_obj_create(ui_ed_obj_t parent);
ui_ed_obj_t plugin_moving_plan_node_ed_obj_create(ui_ed_obj_t parent);
ui_ed_obj_t plugin_moving_plan_segment_ed_obj_create(ui_ed_obj_t parent);
    
#ifdef __cplusplus
}
#endif

#endif 
