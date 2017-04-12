#ifndef UI_PLUGIN_PACKAGE_QUEUE_I_H
#define UI_PLUGIN_PACKAGE_QUEUE_I_H
#include "plugin/package/plugin_package_queue.h"
#include "plugin_package_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_queue {
    plugin_package_module_t m_module;
    cpe_hash_entry m_hh;
    const char * m_name;
    plugin_package_queue_policy_t m_policy;
    uint32_t m_limit;
    plugin_package_group_t m_group;
};

void plugin_package_queue_free_all(plugin_package_module_t module);

uint32_t plugin_package_queue_hash(plugin_package_queue_t package);
int plugin_package_queue_eq(plugin_package_queue_t l, plugin_package_queue_t r);

#ifdef __cplusplus
}
#endif

#endif
