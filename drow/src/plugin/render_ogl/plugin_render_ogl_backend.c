#include <assert.h>
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_backend.h"
#include "plugin_render_ogl_module_i.h"
#include "plugin_render_ogl_program_i.h"
#include "plugin_render_ogl_cache_i.h"

int plugin_render_ogl_module_init_backend(plugin_render_ogl_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_backend_t backend =
            ui_runtime_render_backend_create(
                module->m_runtime, "ogl", module,
                /*render*/
                plugin_render_ogl_render_bind,
                plugin_render_ogl_render_unbind,
                /*camera*/
                plugin_render_ogl_camera_update,
                /*program*/
                sizeof(struct plugin_render_ogl_program),
                plugin_render_ogl_program_init,
                plugin_render_ogl_program_fini,
                /*program attr*/
                sizeof(struct plugin_render_ogl_program_attr),
                plugin_render_ogl_program_attr_init,
                plugin_render_ogl_program_attr_fini,
                /*program unif*/
                sizeof(struct plugin_render_ogl_program_unif),
                plugin_render_ogl_program_unif_init,
                plugin_render_ogl_program_unif_fini,
                /*env*/
                plugin_render_ogl_state_save,
                plugin_render_ogl_state_restore,
                /*commit*/
                plugin_render_ogl_commit_begin,
                plugin_render_ogl_commit_done,
                plugin_render_ogl_commit_group_begin,
                plugin_render_ogl_commit_group_done,
                plugin_render_ogl_commit_cmd);
        if (backend == NULL) {
            CPE_ERROR(module->m_em, "plugin_render_ogl_module_init_backend: create sound backend fail!");
            return -1;
        }
    }
    
    return 0;
}

void plugin_render_ogl_module_fini_backend(plugin_render_ogl_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_backend_t backend = ui_runtime_render_backend_find_by_name(module->m_runtime, "ogl");
        if (backend) {
            ui_runtime_render_backend_free(backend);
        }
    }
}
