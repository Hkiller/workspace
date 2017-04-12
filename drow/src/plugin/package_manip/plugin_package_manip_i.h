#ifndef PLUGIN_PACKAGE_MANIP_I_H
#define PLUGIN_PACKAGE_MANIP_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/xcalc/xcalc_types.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_package_manip_search * plugin_package_manip_search_t;
typedef struct plugin_package_manip_search_map * plugin_package_manip_search_map_t;
    
typedef TAILQ_HEAD(plugin_package_manip_res_collector_list, plugin_package_manip_res_collector) plugin_package_manip_res_collector_list_t;
typedef TAILQ_HEAD(plugin_package_manip_src_convertor_list, plugin_package_manip_src_convertor) plugin_package_manip_src_convertor_list_t;
typedef TAILQ_HEAD(plugin_package_manip_search_map_list, plugin_package_manip_search_map) plugin_package_manip_search_map_list_t;
    
struct plugin_package_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_ed_mgr_t m_ed_mgr;
    plugin_package_module_t m_package_module;
    uint8_t m_debug;
    xcomputer_t m_computer;
    plugin_package_manip_res_collector_list_t m_res_collectors;
    plugin_package_manip_src_convertor_list_t m_src_convertors;
};

#ifdef __cplusplus
}
#endif

#endif 
