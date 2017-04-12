#include "render/runtime/ui_runtime_render_utils.h"
#include "ui_runtime_render_program_state_unif_i.h"
#include "ui_runtime_render_program_unif_i.h"
#include "ui_runtime_render_program_state_i.h"

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_create(
    ui_runtime_render_program_state_t state, ui_runtime_render_program_unif_t unif, ui_runtime_render_program_state_unif_data_t unif_data)
{
    ui_runtime_module_t module = state->m_render->m_module;
    ui_runtime_render_program_state_unif_t program_state_unif;

    if (unif->m_type != unif_data->m_type) {
        CPE_ERROR(
            module->m_em, "ui_runtime_render_program_state_unif_create: unif type masmatch, require %s, but %s!",
            ui_runtime_render_program_unif_type_to_str(unif->m_type),
            ui_runtime_render_program_unif_type_to_str(unif_data->m_type));
        return NULL;
    }
    
    program_state_unif = TAILQ_FIRST(&module->m_free_program_state_unifs);
    if (program_state_unif) {
        TAILQ_REMOVE(&module->m_free_program_state_unifs, program_state_unif, m_next);
    }
    else {
        program_state_unif = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_program_state_unif));
        if (program_state_unif == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_program_state_unif_create: alloc fail!");
            return NULL;
        }
    }

    program_state_unif->m_program_state = state;
    program_state_unif->m_unif = unif;
    program_state_unif->m_data = *unif_data;

    state->m_is_sorted = 0;
    TAILQ_INSERT_TAIL(&state->m_unifs, program_state_unif, m_next);
    return program_state_unif;
}

void ui_runtime_render_program_state_unif_free(ui_runtime_render_program_state_unif_t program_state_unif) {
    ui_runtime_module_t module = program_state_unif->m_program_state->m_render->m_module;

    TAILQ_REMOVE(&program_state_unif->m_program_state->m_unifs, program_state_unif, m_next);

    program_state_unif->m_program_state = (ui_runtime_render_program_state_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_program_state_unifs, program_state_unif, m_next);
}

void ui_runtime_render_program_state_unif_real_free(ui_runtime_render_program_state_unif_t program_state_unif) {
    ui_runtime_module_t module = (ui_runtime_module_t)program_state_unif->m_program_state;
    TAILQ_REMOVE(&module->m_free_program_state_unifs, program_state_unif, m_next);
    mem_free(module->m_alloc, program_state_unif);
}

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_find_by_buildin(
    ui_runtime_render_program_state_t state, ui_runtime_render_program_unif_buildin_t buidin)
{
    ui_runtime_render_program_unif_t unif = ui_runtime_render_program_unif_buildin(state->m_program, buidin);
    return unif ? ui_runtime_render_program_state_unif_find_by_unif(state, unif) : NULL;
}

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_find_by_name(
    ui_runtime_render_program_state_t state, const char * name)
{
    ui_runtime_render_program_state_unif_t state_unif;
    
    TAILQ_FOREACH(state_unif, &state->m_unifs, m_next) {
        if (strcmp(state_unif->m_unif->m_name, name) == 0) return state_unif;
    }

    return NULL;
}

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_find_by_unif(
    ui_runtime_render_program_state_t state, ui_runtime_render_program_unif_t unif)
{
    ui_runtime_render_program_state_unif_t state_unif;
    
    TAILQ_FOREACH(state_unif, &state->m_unifs, m_next) {
        if (state_unif->m_unif == unif) return state_unif;
    }

    return NULL;
}

ui_runtime_render_program_unif_t
ui_runtime_render_program_state_unif_unif(ui_runtime_render_program_state_unif_t unif) {
    return unif->m_unif;
}

ui_runtime_render_program_state_unif_data_t
ui_runtime_render_program_state_unif_data(ui_runtime_render_program_state_unif_t unif) {
    return &unif->m_data;
}

int ui_runtime_render_program_state_unif_data_cmp(
    ui_runtime_render_program_state_unif_data_t l, ui_runtime_render_program_state_unif_data_t r)
{
    if (l->m_type != r->m_type) return l->m_type < r->m_type ? -1 : 1;

    switch(l->m_type) {
    case ui_runtime_render_program_unif_f:
        return l->m_data.m_f == r->m_data.m_f
            ? 0
            : (l->m_data.m_f < r->m_data.m_f
               ? -1
               : 1);
    case ui_runtime_render_program_unif_i:
        return l->m_data.m_i == r->m_data.m_i
            ? 0
            : (l->m_data.m_i < r->m_data.m_i
               ? -1
               : 1);
    case ui_runtime_render_program_unif_v2:
        return ui_vector_2_cmp(&l->m_data.m_v2, &r->m_data.m_v2);
    case ui_runtime_render_program_unif_v3:
        return ui_vector_3_cmp(&l->m_data.m_v3, &r->m_data.m_v3);
    case ui_runtime_render_program_unif_v4:
        return ui_vector_4_cmp(&l->m_data.m_v4, &r->m_data.m_v4);
    case ui_runtime_render_program_unif_m16:
        return ui_transform_cmp(&l->m_data.m_m16, &r->m_data.m_m16);
    case ui_runtime_render_program_unif_texture:
        if (l->m_data.m_tex.m_res != r->m_data.m_tex.m_res) return l->m_data.m_tex.m_res < r->m_data.m_tex.m_res ? -1 : 1;
        if (l->m_data.m_tex.m_min_filter != r->m_data.m_tex.m_min_filter) return l->m_data.m_tex.m_min_filter < r->m_data.m_tex.m_min_filter ? -1 : 1;
        if (l->m_data.m_tex.m_mag_filter != r->m_data.m_tex.m_mag_filter) return l->m_data.m_tex.m_mag_filter < r->m_data.m_tex.m_mag_filter ? -1 : 1;
        if (l->m_data.m_tex.m_wrap_s != r->m_data.m_tex.m_wrap_s) return l->m_data.m_tex.m_wrap_s < r->m_data.m_tex.m_wrap_s ? -1 : 1;
        if (l->m_data.m_tex.m_wrap_t != r->m_data.m_tex.m_wrap_t) return l->m_data.m_tex.m_wrap_t < r->m_data.m_tex.m_wrap_t ? -1 : 1;
        return 0;
    default:
        return 0;
    }
}
