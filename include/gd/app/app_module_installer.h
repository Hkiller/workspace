#ifndef GD_APP_MODULE_INSTALLER_H
#define GD_APP_MODULE_INSTALLER_H
#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_app_module_installer_t gd_app_module_installer_create(gd_app_context_t app, cfg_t cfg);
void gd_app_module_installer_free(gd_app_module_installer_t installer);

uint8_t gd_app_module_installer_is_done(gd_app_module_installer_t installer);
int gd_app_module_installer_install_one(gd_app_module_installer_t installer);
    
#ifdef __cplusplus
}
#endif

#endif
