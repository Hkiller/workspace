#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "ui_runtime_render_program_i.h"
#include "ui_runtime_render_backend_i.h"
#include "ui_runtime_render_program_attr_i.h"
#include "ui_runtime_render_program_unif_i.h"

ui_runtime_render_program_t
ui_runtime_render_program_create(ui_runtime_render_t render, const char * name) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_backend_t backend = module->m_render_backend;
    ui_runtime_render_program_t program;

    assert(backend);
    
    program = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_program) + backend->m_program_capacity);
    if (program == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_program: create sahder program %s: alloc error", name);
        return NULL;
    }

    program->m_render = render;
    cpe_str_dup(program->m_name, sizeof(program->m_name), name);
    TAILQ_INIT(&program->m_attrs);
    TAILQ_INIT(&program->m_unifs);
    program->m_state_count  = 0;
    program->m_attr_flag = 0;
    bzero(&program->m_unif_buildins, sizeof(program->m_unif_buildins));
    
    if (backend->m_program_init) {
        if (backend->m_program_init(backend->m_ctx, program) != 0) {
            CPE_ERROR(module->m_em, "ui_runtime_render_program: create sahder program %s: backend init fail", name);
            mem_free(module->m_alloc, program);
            return NULL;
        }
    }
    
    TAILQ_INSERT_TAIL(&render->m_programs, program, m_next);
    return program;
}

void ui_runtime_render_program_free(ui_runtime_render_program_t  program) {
    ui_runtime_render_program_free_i(program, 0);
}

void ui_runtime_render_program_free_i(ui_runtime_render_program_t  program, uint8_t is_external_unloaded) {
    ui_runtime_render_t render = program->m_render;
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_backend_t backend = module->m_render_backend;

    assert(program->m_state_count == 0);
    assert(backend);
    if (backend->m_program_fini) {
        backend->m_program_fini(backend->m_ctx, program, is_external_unloaded);
    }

    ui_runtime_render_program_clear_attrs_and_unifs(program);

    TAILQ_REMOVE(&program->m_render->m_programs, program, m_next);
    mem_free(module->m_alloc, program);
}

ui_runtime_render_program_t ui_runtime_render_program_find_by_name(ui_runtime_render_t render, const char * name) {
    ui_runtime_render_program_t program;
    
    TAILQ_FOREACH(program, &render->m_programs, m_next) {
        if (strcmp(program->m_name, name) == 0) return program;
    }

    return NULL;
}

const char * ui_runtime_render_program_name(ui_runtime_render_program_t program) {
    return program->m_name;
}

void * ui_runtime_render_program_data(ui_runtime_render_program_t program) {
    return program + 1;
}

ui_runtime_render_program_t ui_runtime_render_program_from_data(void * data) {
    return ((ui_runtime_render_program_t)data) - 1;
}

uint32_t ui_runtime_render_program_attr_flag(ui_runtime_render_program_t program) {
    return program->m_attr_flag;
}

void ui_runtime_render_program_clear_attrs_and_unifs(ui_runtime_render_program_t program) {
    while(!TAILQ_EMPTY(&program->m_attrs)) {
        ui_runtime_render_program_attr_free(TAILQ_FIRST(&program->m_attrs));
    }

    while(!TAILQ_EMPTY(&program->m_unifs)) {
        ui_runtime_render_program_unif_free(TAILQ_FIRST(&program->m_unifs));
    }
}

ui_runtime_render_program_unif_t
ui_runtime_render_program_unif_buildin(
    ui_runtime_render_program_t program, ui_runtime_render_program_unif_buildin_t t)
{
    assert(t < CPE_ARRAY_SIZE(program->m_unif_buildins));
    return program->m_unif_buildins[t];
}

void ui_runtime_render_program_unif_set_buildin(
    ui_runtime_render_program_t program,
    ui_runtime_render_program_unif_buildin_t t, ui_runtime_render_program_unif_t unif)
{
    assert(t < CPE_ARRAY_SIZE(program->m_unif_buildins));
    program->m_unif_buildins[t] = unif;
}

static ui_runtime_render_program_unif_t
ui_runtime_render_program_unif_next(struct ui_runtime_render_program_unif_it * it) {
    ui_runtime_render_program_unif_t * data = (ui_runtime_render_program_unif_t *)(it->m_data);
    ui_runtime_render_program_unif_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_runtime_render_program_unifs(ui_runtime_render_program_t program, ui_runtime_render_program_unif_it_t it) {
    *(ui_runtime_render_program_unif_t *)(it->m_data) = TAILQ_FIRST(&program->m_unifs);
    it->next = ui_runtime_render_program_unif_next;
}

static ui_runtime_render_program_attr_t
ui_runtime_render_program_attr_next(struct ui_runtime_render_program_attr_it * it) {
    ui_runtime_render_program_attr_t * data = (ui_runtime_render_program_attr_t *)(it->m_data);
    ui_runtime_render_program_attr_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_runtime_render_program_attrs(ui_runtime_render_program_t program, ui_runtime_render_program_attr_it_t it) {
    *(ui_runtime_render_program_attr_t *)(it->m_data) = TAILQ_FIRST(&program->m_attrs);
    it->next = ui_runtime_render_program_attr_next;
}
