#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "ui_runtime_render_program_attr_i.h"
#include "ui_runtime_render_program_i.h"
#include "ui_runtime_render_backend_i.h"

ui_runtime_render_program_attr_t
ui_runtime_render_program_attr_create(ui_runtime_render_program_t program, ui_runtime_render_program_attr_id_t attr_id) {
    ui_runtime_module_t module = program->m_render->m_module;
    ui_runtime_render_program_attr_t attr;

    if (module->m_render_backend == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime: create program attr: no backend");
        return NULL;
    }

    if (cpe_ba_get(&program->m_attr_flag, attr_id)) {
        CPE_ERROR(module->m_em, "ui_runtime: create program attr: attr %d duplicate", attr_id);
        return NULL;
    }
               
    attr = mem_calloc(module->m_alloc, sizeof(struct ui_runtime_render_program_attr) + module->m_render_backend->m_program_attr_capacity);
    if (attr == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime: create program attr: alloc error");
        return NULL;
    }

    attr->m_program = program;
    attr->m_attr_id = attr_id;

    if (module->m_render_backend->m_program_attr_init) {
        if (module->m_render_backend->m_program_attr_init(module->m_render_backend->m_ctx, attr) != 0) {
            CPE_ERROR(module->m_em, "ui_runtime: create program attr: init fail");
            mem_free(module->m_alloc, attr);
            return NULL;
        }
    }

    cpe_ba_set(&program->m_attr_flag, attr_id, cpe_ba_true);
    TAILQ_INSERT_TAIL(&program->m_attrs, attr, m_next);

    return attr;
}

void ui_runtime_render_program_attr_free(ui_runtime_render_program_attr_t attr) {
    ui_runtime_module_t module = attr->m_program->m_render->m_module;

    assert(module->m_render_backend);

    if (module->m_render_backend->m_program_attr_fini) {
        module->m_render_backend->m_program_attr_fini(module->m_render_backend->m_ctx, attr);
    }
    
    cpe_ba_set(&attr->m_program->m_attr_flag, attr->m_attr_id, cpe_ba_false);
    TAILQ_REMOVE(&attr->m_program->m_attrs, attr, m_next);

    mem_free(module->m_alloc, attr);
}

ui_runtime_render_program_attr_t
ui_runtime_render_program_attr_find(ui_runtime_render_program_t program, ui_runtime_render_program_attr_id_t attr_id) {
    ui_runtime_render_program_attr_t attr;

    TAILQ_FOREACH(attr, &program->m_attrs, m_next) {
        if (attr->m_attr_id == attr_id) return attr;
    }

    return NULL;
}

void * ui_runtime_render_program_attr_data(ui_runtime_render_program_attr_t attr) {
    return attr + 1;
}

ui_runtime_render_program_attr_id_t ui_runtime_render_program_attr_id(ui_runtime_render_program_attr_t attr) {
    return attr->m_attr_id;
}

const char * ui_runtime_render_program_attr_id_str(ui_runtime_render_program_attr_t attr) {
    return ui_runtime_render_program_attr_id_to_str(attr->m_attr_id);
}
