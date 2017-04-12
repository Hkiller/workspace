#ifndef PLUGIN_PACKAGE_REGION_I_H
#define PLUGIN_PACKAGE_REGION_I_H
#include "plugin/package/plugin_package_region.h"
#include "plugin_package_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_region {
    plugin_package_module_t m_module;
    TAILQ_ENTRY(plugin_package_region) m_next_for_module;
    char m_name[64];
    plugin_package_group_t m_group;
};

#ifdef __cplusplus
}
#endif

#endif
