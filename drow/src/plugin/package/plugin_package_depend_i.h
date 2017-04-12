#ifndef PLUGIN_PACKAGE_DEPEND_I_H
#define PLUGIN_PACKAGE_DEPEND_I_H
#include "plugin_package_package_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_depend {
    plugin_package_package_t m_base_package;
    plugin_package_package_t m_extern_package;
    TAILQ_ENTRY(plugin_package_depend) m_next_for_base;
    TAILQ_ENTRY(plugin_package_depend) m_next_for_extern;
};

plugin_package_depend_t
plugin_package_depend_create(plugin_package_package_t base_package, plugin_package_package_t extern_package);
void plugin_package_depend_free(plugin_package_depend_t depend);
    
#ifdef __cplusplus
}
#endif

#endif
