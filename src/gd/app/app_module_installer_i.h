#ifndef GD_APP_MODULE_INSTALLER_I_H
#define GD_APP_MODULE_INSTALLER_I_H
#include "gd/app/app_module_installer.h"
#include "app_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gd_app_module_installer_stack_node {
    cfg_t m_cfg;
    struct cfg_it m_childs_it;
};
    
struct gd_app_module_installer {
    gd_app_context_t m_app;
    cfg_t m_next;
    uint8_t m_stack_size;
    int m_rv;
    struct gd_app_module_installer_stack_node m_stack[32];
};

#ifdef __cplusplus
}
#endif

#endif
