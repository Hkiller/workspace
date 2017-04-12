#ifndef UI_PLUGIN_PACKAGE_INSTALLER_H
#define UI_PLUGIN_PACKAGE_INSTALLER_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_package_installer_start_fun_t)(void * ctx, plugin_package_package_t package);
typedef void (*plugin_package_installer_cancel_fun_t)(void * ctx, plugin_package_package_t package);
    
plugin_package_installer_t
plugin_package_installer_create(
    plugin_package_module_t module,
    void * ctx,
    plugin_package_installer_start_fun_t start_fun,
    plugin_package_installer_cancel_fun_t cancel_fun);
    
void plugin_package_installer_free(plugin_package_installer_t installer);

plugin_package_installer_t plugin_package_installer_get(plugin_package_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif

