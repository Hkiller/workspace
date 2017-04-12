#ifndef PLUGIN_PACKAGE_INSTALLER_I_H
#define PLUGIN_PACKAGE_INSTALLER_I_H
#include "plugin/package/plugin_package_installer.h"
#include "plugin_package_package_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_installer {
    plugin_package_module_t m_module;
    void * m_ctx;
    plugin_package_installer_start_fun_t m_start_fun;
    plugin_package_installer_cancel_fun_t m_cancel_fun;
};
    
#ifdef __cplusplus
}
#endif

#endif
