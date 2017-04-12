#ifndef PLUGIN_PACKAGE_GROUP_I_H
#define PLUGIN_PACKAGE_GROUP_I_H
#include "plugin/package/plugin_package_group.h"
#include "plugin_package_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_group {
    plugin_package_module_t m_module;
    char m_name[64];
    TAILQ_ENTRY(plugin_package_group) m_next_for_module;
    plugin_package_package_using_state_t m_package_using_state;
    uint32_t m_package_count;
    plugin_package_group_ref_list_t m_packages;
};

void plugin_package_group_real_free(plugin_package_group_t group);

#ifdef __cplusplus
}
#endif

#endif
