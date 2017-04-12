#ifndef PLUGIN_PACKAGE_GROUP_REF_I_H
#define PLUGIN_PACKAGE_GROUP_REF_I_H
#include "plugin_package_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_group_ref {
    plugin_package_group_t m_group;
    plugin_package_package_t m_package;
    TAILQ_ENTRY(plugin_package_group_ref) m_next_for_group;
    TAILQ_ENTRY(plugin_package_group_ref) m_next_for_package;
};

plugin_package_group_ref_t plugin_package_group_ref_create(plugin_package_group_t group, plugin_package_package_t package);
void plugin_package_group_ref_free(plugin_package_group_ref_t ref);
void plugin_package_group_ref_real_free(plugin_package_group_ref_t ref);
    
#ifdef __cplusplus
}
#endif

#endif
