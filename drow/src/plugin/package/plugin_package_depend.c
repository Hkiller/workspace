#include <assert.h>
#include "plugin_package_depend_i.h"
#include "plugin_package_package_i.h"

plugin_package_depend_t
plugin_package_depend_create(plugin_package_package_t base_package, plugin_package_package_t extern_package) {
    plugin_package_depend_t depend;
    plugin_package_module_t module = base_package->m_module;

    depend = mem_alloc(module->m_alloc, sizeof(struct plugin_package_depend));
    if (depend == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_depend alloc fail!");
        return NULL;
    }

    depend->m_base_package = base_package;
    depend->m_extern_package = extern_package;

    TAILQ_INSERT_TAIL(&base_package->m_extern_packages, depend, m_next_for_base);
    TAILQ_INSERT_TAIL(&extern_package->m_base_packages, depend, m_next_for_extern);

    return depend;
}

void plugin_package_depend_free(plugin_package_depend_t depend) {
    plugin_package_module_t module = depend->m_base_package->m_module;

    TAILQ_REMOVE(&depend->m_base_package->m_extern_packages, depend, m_next_for_base);
    TAILQ_REMOVE(&depend->m_extern_package->m_base_packages, depend, m_next_for_extern);

    mem_free(module->m_alloc, depend);
}

