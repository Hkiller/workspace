#include <assert.h>
#include "ui_runtime_render_program_state_attr_i.h"
#include "ui_runtime_render_program_attr_i.h"

ui_runtime_render_program_state_attr_t
ui_runtime_render_program_state_attr_create(
    ui_runtime_render_program_state_t state, ui_runtime_render_program_attr_t attr,
    uint8_t stride,
    uint8_t start_pos,
    uint8_t element_count,
    uint8_t element_type)
{
    ui_runtime_module_t module = state->m_render->m_module;
    ui_runtime_render_program_state_attr_t program_state_attr;

    program_state_attr = TAILQ_FIRST(&module->m_free_program_state_attrs);
    if (program_state_attr) {
        TAILQ_REMOVE(&module->m_free_program_state_attrs, program_state_attr, m_next);
    }
    else {
        program_state_attr = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_program_state_attr));
        if (program_state_attr == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_program_state_attr_create: alloc fail!");
            return NULL;
        }
    }

    assert(attr);
    assert(attr->m_attr_id < ui_runtime_render_program_attr_max);

    program_state_attr->m_program_state = state;
    program_state_attr->m_attr = attr;
    program_state_attr->m_data.m_stride = stride;
    program_state_attr->m_data.m_start_pos = start_pos;
    program_state_attr->m_data.m_element_count = element_count;
    program_state_attr->m_data.m_element_type = element_type;

    state->m_is_sorted = 0;
    TAILQ_INSERT_TAIL(&state->m_attrs, program_state_attr, m_next);
    
    return program_state_attr;
}

int ui_runtime_render_program_state_attr_create_by_id_if_exist(
    ui_runtime_render_program_state_t state,
    ui_runtime_render_program_attr_id_t attr_id,
    uint8_t stride,
    uint8_t start_pos,
    uint8_t element_count,
    uint8_t element_type)
{
    ui_runtime_render_program_attr_t attr = ui_runtime_render_program_attr_find(state->m_program, attr_id);

    if (attr == NULL) return 0;
    
    return ui_runtime_render_program_state_attr_create(state, attr, stride, start_pos, element_count, element_type)
        ? 0 : -1;
}

void ui_runtime_render_program_state_attr_free(ui_runtime_render_program_state_attr_t program_state_attr) {
    ui_runtime_module_t module = program_state_attr->m_program_state->m_render->m_module;

    TAILQ_REMOVE(&program_state_attr->m_program_state->m_attrs, program_state_attr, m_next);

    program_state_attr->m_program_state = (ui_runtime_render_program_state_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_program_state_attrs, program_state_attr, m_next);
}

void ui_runtime_render_program_state_attr_real_free(ui_runtime_render_program_state_attr_t program_state_attr) {
    ui_runtime_module_t module = (ui_runtime_module_t)program_state_attr->m_program_state;
    TAILQ_REMOVE(&module->m_free_program_state_attrs, program_state_attr, m_next);
    mem_free(module->m_alloc, program_state_attr);
}

ui_runtime_render_program_attr_t
ui_runtime_render_program_state_attr_attr(ui_runtime_render_program_state_attr_t attr) {
    return attr->m_attr;
}

ui_runtime_render_program_state_attr_data_t
ui_runtime_render_program_state_attr_data(ui_runtime_render_program_state_attr_t attr) {
    return &attr->m_data;
}

int ui_runtime_render_program_state_attr_data_cmp(ui_runtime_render_program_state_attr_data_t l, ui_runtime_render_program_state_attr_data_t r) {
    if (l->m_stride != r->m_stride) return l->m_stride < r->m_stride ? -1 : 1;
    if (l->m_start_pos != r->m_start_pos) return l->m_start_pos < r->m_start_pos ? -1 : 1;
    if (l->m_element_count != r->m_element_count) return l->m_element_count < r->m_element_count ? -1 : 1;
    if (l->m_element_type != r->m_element_type) return l->m_element_type < r->m_element_type ? -1 : 1;
    return 0;
}
