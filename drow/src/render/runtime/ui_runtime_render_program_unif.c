#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "render/utils/ui_vector_4.h"
#include "render/utils/ui_matrix_3x3.h"
#include "render/utils/ui_matrix_4x4.h"
#include "ui_runtime_render_program_unif_i.h"
#include "ui_runtime_render_backend_i.h"

ui_runtime_render_program_unif_t
ui_runtime_render_program_unif_create(ui_runtime_render_program_t program, const char * name, ui_runtime_render_program_unif_type_t type) {
    ui_runtime_module_t module = program->m_render->m_module;
    ui_runtime_render_program_unif_t unif;

    if (module->m_render_backend == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime: create program unif: no backend");
        return NULL;
    }
    
    unif = mem_calloc(module->m_alloc, sizeof(struct ui_runtime_render_program_unif) + module->m_render_backend->m_program_unif_capacity);
    if (unif == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime: create program unif: alloc error");
        return NULL;
    }

    unif->m_program = program;
    cpe_str_dup(unif->m_name, sizeof(unif->m_name), name);
    unif->m_type = type;
    
    if (module->m_render_backend->m_program_unif_init) {
        if (module->m_render_backend->m_program_unif_init(module->m_render_backend->m_ctx, unif) != 0) {
            CPE_ERROR(module->m_em, "ui_runtime: create program unif: init fail");
            mem_free(module->m_alloc, unif);
            return NULL;
        }
    }

    if (strcmp(name, "matrixMVP") == 0) {
        ui_runtime_render_program_unif_set_buildin(program, ui_runtime_render_program_unif_matrix_mvp, unif);
    }
    else if (strcmp(name, "matrixMV") == 0) {
        ui_runtime_render_program_unif_set_buildin(program, ui_runtime_render_program_unif_matrix_mv, unif);
    }
    else if (strcmp(name, "matrixP") == 0) {
        ui_runtime_render_program_unif_set_buildin(program, ui_runtime_render_program_unif_matrix_p, unif);
    }
    
    TAILQ_INSERT_TAIL(&program->m_unifs, unif, m_next);

    return unif;
}

void ui_runtime_render_program_unif_free(ui_runtime_render_program_unif_t unif) {
    ui_runtime_module_t module = unif->m_program->m_render->m_module;
    uint8_t i;
    
    assert(module->m_render_backend);

    if (module->m_render_backend->m_program_unif_fini) {
        module->m_render_backend->m_program_unif_fini(module->m_render_backend->m_ctx, unif);
    }

    for(i = 0; i < CPE_ARRAY_SIZE(unif->m_program->m_unif_buildins); ++i) {
        if (unif->m_program->m_unif_buildins[i] == unif) {
            unif->m_program->m_unif_buildins[i] = NULL;
            break;
        }
    }
    
    TAILQ_REMOVE(&unif->m_program->m_unifs, unif, m_next);

    mem_free(module->m_alloc, unif);
}

const char * ui_runtime_render_program_unif_name(ui_runtime_render_program_unif_t unif) {
    return unif->m_name;
}

ui_runtime_render_program_unif_t
ui_runtime_render_program_unif_find(ui_runtime_render_program_t program, const char * name) {
    ui_runtime_render_program_unif_t unif;

    TAILQ_FOREACH(unif, &program->m_unifs, m_next) {
        if (strcmp(unif->m_name, name) == 0) return unif;
    }

    return NULL;
}

ui_runtime_render_program_unif_type_t
ui_runtime_render_program_unif_type(ui_runtime_render_program_unif_t unif) {
    return unif->m_type;
}

void * ui_runtime_render_program_unif_data(ui_runtime_render_program_unif_t unif) {
    return unif + 1;
}
