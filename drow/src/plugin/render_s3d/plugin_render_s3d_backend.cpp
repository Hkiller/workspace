#include <assert.h>
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_backend.h"
#include "plugin_render_s3d_module_i.hpp"
#include "plugin_render_s3d_cache_i.hpp"
#include "plugin_render_s3d_program_i.hpp"

int plugin_render_s3d_module_init_backend(plugin_render_s3d_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_backend_t backend =
            ui_runtime_render_backend_create(
                module->m_runtime, "s3d", module,
                /*render*/
                plugin_render_s3d_render_bind,
                plugin_render_s3d_render_unbind,
                /*camera*/
                plugin_render_s3d_camera_update,
                /*program*/
                sizeof(struct plugin_render_s3d_program),
                plugin_render_s3d_program_init,
                plugin_render_s3d_program_fini,
                /*program attr*/
                sizeof(struct plugin_render_s3d_program_attr),
                plugin_render_s3d_program_attr_init,
                plugin_render_s3d_program_attr_fini,
                /*program unif*/
                sizeof(struct plugin_render_s3d_program_unif),
                plugin_render_s3d_program_unif_init,
                plugin_render_s3d_program_unif_fini,
                /*env*/
                plugin_render_s3d_state_save,
                plugin_render_s3d_state_restore,
                /*commit*/
                plugin_render_s3d_commit_begin,
                plugin_render_s3d_commit_done,
                plugin_render_s3d_commit_group_begin,
                plugin_render_s3d_commit_group_done,
                plugin_render_s3d_commit_cmd);
        if (backend == NULL) {
            CPE_ERROR(module->m_em, "plugin_render_s3d_module_init_backend: create sound backend fail!");
            return -1;
        }
    }
    
    return 0;
}

void plugin_render_s3d_module_fini_backend(plugin_render_s3d_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_backend_t backend = ui_runtime_render_backend_find_by_name(module->m_runtime, "s3d");
        if (backend) {
            ui_runtime_render_backend_free(backend);
        }
    }
}
