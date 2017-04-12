#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "ui_runtime_render_i.h"

static ui_transform_t ui_runtime_render_matrix_stach_push(ui_runtime_render_t render, struct ui_runtime_render_matrix_stack * stack);

int ui_runtime_render_matrix_push(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type) {
    struct ui_runtime_render_matrix_stack * stack;
    ui_transform_t m;
    
    assert(type < CPE_ARRAY_SIZE(render->m_matrix_stacks));

    stack = render->m_matrix_stacks + type;

    m = ui_runtime_render_matrix_stach_push(render, stack);
    if (m == NULL) return -1;

    assert(stack->m_size > 0);
    if (stack->m_size > 0) *m = *(m - 1);

    return 0;
}

void ui_runtime_render_matrix_pop(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type) {
    struct ui_runtime_render_matrix_stack * stack;
    
    assert(type < CPE_ARRAY_SIZE(render->m_matrix_stacks));

    stack = render->m_matrix_stacks + type;

    assert(stack->m_size > 0);
    if (stack->m_size > 0) stack->m_size--;
}

void ui_runtime_render_matrix_load_identity(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type) {
    struct ui_runtime_render_matrix_stack * stack;
    ui_transform_t m;
    
    assert(type < CPE_ARRAY_SIZE(render->m_matrix_stacks));

    stack = render->m_matrix_stacks + type;

    m = ui_runtime_render_matrix(render, type);
    assert(m);
    *m = UI_TRANSFORM_IDENTITY;
}

void ui_runtime_render_matrix_load(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type, ui_transform_t mat) {
    struct ui_runtime_render_matrix_stack * stack;
    ui_transform_t m;
    
    assert(type < CPE_ARRAY_SIZE(render->m_matrix_stacks));

    stack = render->m_matrix_stacks + type;

    m = ui_runtime_render_matrix(render, type);
    assert(m);
    *m = *mat;
}

void ui_runtime_render_matrix_multiply(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type, ui_transform_t mat) {
    struct ui_runtime_render_matrix_stack * stack;
    ui_transform_t m;
    ui_transform buf;
    
    assert(type < CPE_ARRAY_SIZE(render->m_matrix_stacks));

    stack = render->m_matrix_stacks + type;

    m = ui_runtime_render_matrix(render, type);
    assert(m);

    buf = *m;
    m = mat;
    ui_transform_adj_by_parent(m, &buf);
}

ui_transform_t ui_runtime_render_matrix(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type) {
    struct ui_runtime_render_matrix_stack * stack;
    
    assert(type < CPE_ARRAY_SIZE(render->m_matrix_stacks));

    stack = render->m_matrix_stacks + type;
    return stack->m_size > 0 ? stack->m_buf + (stack->m_size - 1) : NULL;
}

int ui_runtime_render_matrix_reset(ui_runtime_render_t render) {
    int rv = 0;
    
    uint8_t i;
    for(i = 0; i < CPE_ARRAY_SIZE(render->m_matrix_stacks); ++i) {
        struct ui_runtime_render_matrix_stack * stack = render->m_matrix_stacks + i;
        ui_transform_t m;
        
        stack->m_size = 0;
        m = ui_runtime_render_matrix_stach_push(render, stack);
        if (m == NULL) {
            rv = -1;
            continue;
        }

        *m = UI_TRANSFORM_IDENTITY;
    }

    return rv;
}

static ui_transform_t
ui_runtime_render_matrix_stach_push(ui_runtime_render_t render, struct ui_runtime_render_matrix_stack * stack) {
    if (stack->m_size >= stack->m_capacity) {
        uint32_t new_capacity = stack->m_capacity < 16 ? 16 : stack->m_capacity * 2;
        ui_transform_t new_buf = mem_alloc(render->m_module->m_alloc, sizeof(ui_transform) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(render->m_module->m_em, "ui_runtime_render_matrix_stach_push: allock buf fail, capacity=%d", new_capacity);
            return NULL;
        }

        if (stack->m_buf) {
            memcpy(new_buf, stack->m_buf, sizeof(ui_transform) * stack->m_size);
            mem_free(render->m_module->m_alloc, stack->m_buf);
        }

        stack->m_buf = new_buf;
        stack->m_capacity = new_capacity;
    }

    return &stack->m_buf[stack->m_size++];
}
