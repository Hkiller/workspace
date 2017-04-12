#ifndef PLUGIN_SPINE_MANIP_I_H
#define PLUGIN_SPINE_MANIP_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/spine_manip/plugin_spine_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_ed_mgr_t m_ed_mgr;
    plugin_spine_module_t m_spine_module;
    uint8_t m_debug;
};

int plugin_spine_ed_src_load(ui_ed_src_t src);

int plugin_spine_manip_skeleton_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_spine_manip_state_def_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif 
