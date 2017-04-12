#ifndef PLUGIN_SCROLLMAP_MANIP_I_H
#define PLUGIN_SCROLLMAP_MANIP_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/scrollmap_manip/plugin_scrollmap_manip.h"
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_ed_mgr_t m_ed_mgr;
    plugin_package_manip_t m_package_manip;
    plugin_scrollmap_module_t m_scrollmap_module;
    uint8_t m_debug;
};

int plugin_scrollmap_manip_ed_regist(plugin_scrollmap_manip_t module);
void plugin_scrollmap_manip_ed_unregist(plugin_scrollmap_manip_t module);

int plugin_scrollmap_manip_src_convertor_regist(plugin_scrollmap_manip_t module);
void plugin_scrollmap_manip_src_convertor_unregist(plugin_scrollmap_manip_t module);
    
#ifdef __cplusplus
}
#endif

#endif 
