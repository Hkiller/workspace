#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "ui_runtime_render_backend_i.h"
#include "ui_runtime_render_i.h"

ui_runtime_render_backend_t
ui_runtime_render_backend_create(
    ui_runtime_module_t module, const char * name,
    void * ctx,
    /*render*/
    ui_runtime_render_bind_fun_t render_bind,
    ui_runtime_render_unbind_fun_t render_unbind,
    /*camera*/
    ui_runtime_render_camera_update_fun_t camera_update,
    /*program*/
    uint32_t program_capacity,
    ui_runtime_render_program_init_fun_t program_init,
    ui_runtime_render_program_fini_fun_t program_fini,
    /*program attr*/
    uint32_t program_attr_capacity,
    ui_runtime_render_program_attr_init_fun_t program_attr_init,
    ui_runtime_render_program_attr_fini_fun_t program_attr_fini,
    /*program unif*/
    uint32_t program_unif_capacity,
    ui_runtime_render_program_unif_init_fun_t program_unif_init,
    ui_runtime_render_program_unif_fini_fun_t program_unif_fini,
    /*env*/
    ui_runtime_render_state_save_fun_t state_save,
    ui_runtime_render_state_restore_fun_t state_restore,
    /*commit*/
    ui_runtime_render_commit_begin_fun_t commit_being,
    ui_runtime_render_commit_done_fun_t commit_done,
    ui_runtime_render_commit_group_begin_fun_t commit_group_being,
    ui_runtime_render_commit_group_done_fun_t commit_group_done,
    ui_runtime_render_commit_cmd_fun_t commit_cmd)
{
    ui_runtime_render_backend_t backend;
    
    if (module->m_render_backend != NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_backend_create: render backend already exist!!");
        return NULL;
    }
    
    backend = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_backend_create: alloc fail!");
        return NULL;
    }

    cpe_str_dup(backend->m_name, sizeof(backend->m_name), name);
    backend->m_module = module;
    backend->m_ctx = ctx;
    backend->m_render_bind = render_bind;
    backend->m_render_unbind = render_unbind;
    backend->m_camera_update = camera_update;
    backend->m_program_capacity = program_capacity;
    backend->m_program_init = program_init;
    backend->m_program_fini = program_fini;
    backend->m_program_attr_capacity = program_attr_capacity;
    backend->m_program_attr_init = program_attr_init;
    backend->m_program_attr_fini = program_attr_fini;
    backend->m_program_unif_capacity = program_unif_capacity;
    backend->m_program_unif_init = program_unif_init;
    backend->m_program_unif_fini = program_unif_fini;
    backend->m_state_save = state_save;
    backend->m_state_restore = state_restore;
    backend->m_commit_being = commit_being;
    backend->m_commit_done = commit_done;
    backend->m_commit_group_being = commit_group_being;
    backend->m_commit_group_done = commit_group_done;
    backend->m_commit_cmd = commit_cmd;

    module->m_render_backend = backend;

    if (module->m_render) {
        if (ui_runtime_render_init(module->m_render) != 0) {
            module->m_render_backend = NULL;
            mem_free(module->m_alloc, backend);
            return NULL;
        }
    }
    
    return backend;
}
    
void ui_runtime_render_backend_free(ui_runtime_render_backend_t backend) {
    ui_runtime_module_t module = backend->m_module;
    
    assert(module->m_render_backend == backend);

    if (module->m_render && module->m_render->m_inited) {
        ui_runtime_render_fini(module->m_render, 0);
    }
    
    backend->m_module->m_render_backend = NULL;
    mem_free(backend->m_module->m_alloc, backend);
}

ui_runtime_render_backend_t
ui_runtime_render_backend_find_by_name(ui_runtime_module_t module, const char * name) {
    if (module->m_render_backend && strcmp(module->m_render_backend->m_name, name) == 0) {
        return module->m_render_backend;
    }
    return NULL;
}
