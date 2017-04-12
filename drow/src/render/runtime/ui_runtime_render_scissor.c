#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "ui_runtime_render_i.h"

int ui_runtime_render_scissor_push(ui_runtime_render_t render, ui_rect_t scissor) {
    ui_rect_t queue_scissor;
    
    if (render->m_scissor_size >= render->m_scissor_capacity) {
        uint32_t new_capacity = render->m_scissor_capacity < 16 ? 16 : render->m_scissor_capacity * 2;
        ui_rect_t new_buf = mem_alloc(render->m_module->m_alloc, sizeof(struct ui_rect) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(render->m_module->m_em, "ui_runtime_render_scissor_push: allock buf fail, capacity=%d", new_capacity);
            return -1;
        }

        if (render->m_scissor_buf) {
            memcpy(new_buf, render->m_scissor_buf, sizeof(struct ui_rect) * render->m_scissor_size);
            mem_free(render->m_module->m_alloc, render->m_scissor_buf);
        }

        render->m_scissor_buf = new_buf;
        render->m_scissor_capacity = new_capacity;
    }

    queue_scissor = &render->m_scissor_buf[render->m_scissor_size++];

    queue_scissor->lt.x = scissor->lt.x;
    queue_scissor->lt.y = render->m_view_size.y - scissor->rb.y;
    queue_scissor->rb.x = scissor->rb.x;
    queue_scissor->rb.y = render->m_view_size.y - scissor->lt.y;
    
    ui_runtime_render_state_set_scissor(render->m_render_state, queue_scissor);
    
    return 0;
}

void ui_runtime_render_scissor_pop(ui_runtime_render_t render) {
    assert(render->m_scissor_size > 0);
    render->m_scissor_size--;
    
    if (render->m_scissor_size > 0) {
        ui_runtime_render_state_set_scissor(render->m_render_state, render->m_scissor_buf + render->m_scissor_size);
    }
    else {
        ui_runtime_render_state_set_scissor(render->m_render_state, NULL);
    }
}
