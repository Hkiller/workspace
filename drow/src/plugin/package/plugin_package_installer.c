#include <assert.h>
#include "plugin_package_installer_i.h"

plugin_package_installer_t
plugin_package_installer_create(
    plugin_package_module_t module,
    void * ctx,
    plugin_package_installer_start_fun_t start_fun,
    plugin_package_installer_cancel_fun_t cancel_fun)
{
    plugin_package_installer_t installer;

    if (module->m_installer) {
        CPE_ERROR(module->m_em, "plugin_package_installer: create: already have installer!");
        return NULL;
    }
    
    installer = mem_alloc(module->m_alloc, sizeof(struct plugin_package_installer));
    if (installer == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_installer: create: alloc fail!");
        return NULL;
    }

    installer->m_module = module;
    installer->m_ctx = ctx;
    installer->m_start_fun = start_fun;
    installer->m_cancel_fun = cancel_fun;
    
    module->m_installer = installer;
    return installer;
}

void plugin_package_installer_free(plugin_package_installer_t installer) {
    plugin_package_module_t module = installer->m_module;

    assert(module->m_installer == installer);

    module->m_installer = NULL;
    mem_free(module->m_alloc, installer);
}

plugin_package_installer_t plugin_package_installer_get(plugin_package_module_t module) {
    return module->m_installer;
}

