#include "cpe/vfs/vfs_manage.h"
#include "cpe/vfs/vfs_mount_point.h"
#include "gd/app/app_context.h"
#include "plugin_app_env_backend_i.hpp"

// static struct {
//     const char * name;
//     int (*init)(plugin_app_env_module_t module);
//     void (*fini)(plugin_app_env_module_t module);
// } s_auto_reg_products[] = {
//     { "vfs-backend", ui_app_android_vfs_backend_init, ui_app_android_vfs_backend_fini }
//     , { "vfs-mount", ui_app_android_vfs_backend_mount, ui_app_android_vfs_backend_unmount }
// };
plugin_app_env_backend_t g_app_env_flex = NULL;

int plugin_app_env_backend_init(plugin_app_env_module_t module) {
    plugin_app_env_backend_t backend = (plugin_app_env_backend_t)mem_alloc(module->m_alloc, sizeof(struct plugin_app_env_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env_backend: alloc backend fail!");
        return -1;
    }

    vfs_mgr_t vfs = gd_app_vfs_mgr(module->m_app);
    vfs_mount_point_set_bridger_to(vfs_mgr_current_point(vfs), vfs_mgr_root_point(vfs));
    
    backend->m_module = module;
    
    // for(uint8_t component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
    //     if (s_auto_reg_products[component_pos].init(module) != 0) {
    //         CPE_ERROR(module->m_em, "plugin_app_env_backend_init: regist product %s fail!", s_auto_reg_products[component_pos].name);
    //         for(; component_pos > 0; component_pos--) {
    //             s_auto_reg_products[component_pos - 1].fini(module);
    //         }
    //         return -1;
    //     }
    // }

    g_app_env_flex = backend;
    module->m_backend = backend;
    
    return 0;
}

void plugin_app_env_backend_fini(plugin_app_env_module_t module) {
    // for(uint8_t component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
    //     s_auto_reg_products[component_pos - 1].fini(module);
    // }

    vfs_mgr_t vfs = gd_app_vfs_mgr(module->m_app);
    vfs_mount_point_set_bridger_to(vfs_mgr_current_point(vfs), NULL);
    
    mem_free(module->m_alloc, module->m_backend);
    module->m_backend = NULL;
    g_app_env_flex = NULL;
}

