#ifndef PLUGIN_PACKAGE_MANIP_RES_COLLECTOR_I_H
#define PLUGIN_PACKAGE_MANIP_RES_COLLECTOR_I_H
#include "plugin/package_manip/plugin_package_manip_res_collector.h"
#include "plugin_package_manip_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_manip_res_collector {
    plugin_package_manip_t m_manip;
    TAILQ_ENTRY(plugin_package_manip_res_collector) m_next;
    char m_name[32];
    plugin_package_manip_res_collector_fun_t m_fun;
    void * m_ctx;
};

int plugin_package_manip_create_res_collector_src(plugin_package_manip_t manip);
int plugin_package_manip_create_res_collector_res(plugin_package_manip_t manip);
int plugin_package_manip_create_res_collector_package(plugin_package_manip_t manip);
int plugin_package_manip_create_res_collector_region(plugin_package_manip_t manip);
int plugin_package_manip_create_res_collector_extern_shared(plugin_package_manip_t manip);

#ifdef __cplusplus
}
#endif

#endif 
