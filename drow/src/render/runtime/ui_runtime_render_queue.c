#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/tailq_sort.h"
#include "ui_runtime_render_queue_i.h"
#include "ui_runtime_render_backend_i.h"
#include "ui_runtime_render_cmd_i.h"

ui_runtime_render_queue_t
ui_runtime_render_queue_create(ui_runtime_render_t render) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_queue_t queue;
    uint32_t i;

    if (module->m_render_backend == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_queue: no backend!");
        return NULL;
    }
    
    queue = TAILQ_FIRST(&render->m_queues);
    if (queue) {
        TAILQ_REMOVE(&module->m_free_queues, queue, m_next);
    }
    else {
        queue = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_queue));
        if (queue == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_queue: alloc fail!");
            return NULL;
        }
    }
    
    queue->m_render = render;
    for(i = 0; i < CPE_ARRAY_SIZE(queue->m_groups); ++i) {
        TAILQ_INIT(&queue->m_groups[i]);
    }

    render->m_queue_count++;
    TAILQ_INSERT_TAIL(&render->m_queues, queue, m_next);
    return queue;
}

void ui_runtime_render_queue_free(ui_runtime_render_queue_t queue) {
    ui_runtime_render_t render = queue->m_render;
    ui_runtime_module_t module = render->m_module;
    uint32_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(queue->m_groups); ++i) {
        ui_runtime_render_cmd_list_t * cmd_list = &queue->m_groups[i];
        while(!TAILQ_EMPTY(cmd_list)) {
            ui_runtime_render_cmd_free(TAILQ_FIRST(cmd_list));
        }
    }

    if (render->m_default_queue == queue) render->m_default_queue = NULL;

    assert(render->m_queue_count > 0);
    render->m_queue_count--;
    TAILQ_REMOVE(&queue->m_render->m_queues, queue, m_next);

    /*release*/
    queue->m_render = (ui_runtime_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_queues, queue, m_next);
}

void ui_runtime_render_queue_real_free(ui_runtime_render_queue_t queue) {
    ui_runtime_module_t module = (ui_runtime_module_t)queue->m_render;
    TAILQ_REMOVE(&module->m_free_queues, queue, m_next);
    mem_free(module->m_alloc, queue);
}

int ui_runtime_render_queue_push(ui_runtime_render_queue_t queue) {
    ui_runtime_render_t render = queue->m_render;
    ui_runtime_module_t module = render->m_module;

    if (render->m_queue_stack_count >= render->m_queue_stack_capacity) {
        uint32_t new_capacity = render->m_queue_stack_capacity > 16 ? render->m_queue_stack_capacity * 2 : 16;
        ui_runtime_render_queue_t * new_buf = mem_alloc(module->m_alloc, sizeof(ui_runtime_render_queue_t) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_queue_push: alloc buf fail, capacity=%d!", new_capacity);
            return -1;
        }

        if (render->m_queue_stack) {
            assert(render->m_queue_stack_count > 0);
            memcpy(new_buf, render->m_queue_stack, sizeof(ui_runtime_render_queue_t) * render->m_queue_stack_count);
            mem_free(module->m_alloc, render->m_queue_stack);
        }

        render->m_queue_stack = new_buf;
        render->m_queue_stack_capacity = new_capacity;
    }

    render->m_queue_stack[render->m_queue_stack_count++] = queue;
    
    return 0;
}

void ui_runtime_render_queue_pop(ui_runtime_render_queue_t queue) {
    assert(queue == ui_runtime_render_queue_top(queue->m_render));

    assert(queue->m_render->m_queue_stack_count > 0);
    queue->m_render->m_queue_stack_count--;
}

ui_runtime_render_queue_t
ui_runtime_render_queue_top(ui_runtime_render_t render) {
    return render->m_queue_stack_count > 0 ? render->m_queue_stack[render->m_queue_stack_count - 1] : NULL;
}

static int ui_runtime_render_cmd_by_depth(ui_runtime_render_cmd_t l, ui_runtime_render_cmd_t r, void * p) {
    return l->m_depth < r->m_depth
        ? 1
        : (l->m_depth > r->m_depth
           ? -1
           : 0);
}

static int ui_runtime_render_cmd_by_logic_z(ui_runtime_render_cmd_t l, ui_runtime_render_cmd_t r, void * p) {
    return l->m_logic_z < r->m_logic_z
        ? -1
        : (l->m_logic_z > r->m_logic_z
           ? 1
           : 0);
}

void ui_runtime_render_queue_sort(ui_runtime_render_queue_t queue) {
    /* Don't sort _queue0, it already comes sorted */
    TAILQ_SORT(&queue->m_groups[ui_runtime_render_queue_group_3d_transparent], ui_runtime_render_cmd, ui_runtime_render_cmd_list, m_next, ui_runtime_render_cmd_by_depth, NULL);
    TAILQ_SORT(&queue->m_groups[ui_runtime_render_queue_group_neg], ui_runtime_render_cmd, ui_runtime_render_cmd_list, m_next, ui_runtime_render_cmd_by_logic_z, NULL);
    TAILQ_SORT(&queue->m_groups[ui_runtime_render_queue_group_pos], ui_runtime_render_cmd, ui_runtime_render_cmd_list, m_next, ui_runtime_render_cmd_by_logic_z, NULL);
}

void ui_runtime_render_queue_clear(ui_runtime_render_queue_t queue) {
    size_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(queue->m_groups); ++i) {
        ui_runtime_render_cmd_list_t * cmd_list = &queue->m_groups[i];
        while(!TAILQ_EMPTY(cmd_list)) {
            ui_runtime_render_cmd_free(TAILQ_FIRST(cmd_list));
        }
    }
}

void ui_runtime_render_queue_state_save(ui_runtime_render_queue_t queue) {
    ui_runtime_module_t module = queue->m_render->m_module;
    bzero(&queue->m_saved_state, sizeof(queue->m_saved_state));
    if (module->m_render_backend->m_state_save) {
        module->m_render_backend->m_state_save(module->m_render_backend->m_ctx, &queue->m_saved_state);
    }
}

void ui_runtime_render_queue_state_restore(ui_runtime_render_queue_t queue) {
    ui_runtime_module_t module = queue->m_render->m_module;
    if (module->m_render_backend->m_state_restore) {
        module->m_render_backend->m_state_restore(module->m_render_backend->m_ctx, &queue->m_saved_state);
    }
}

ui_runtime_render_queue_group_t
ui_runtime_render_queue_select_type(float logic_z, uint8_t is_3d, uint8_t is_transparent) {
    if(logic_z < 0.0f) {
        return ui_runtime_render_queue_group_neg;
    }
    else if(logic_z > 0.0f) {
        return ui_runtime_render_queue_group_pos;
    }
    else {
        if(is_3d) {
            if(is_transparent) {
                return ui_runtime_render_queue_group_3d_transparent;
            }
            else {
                return ui_runtime_render_queue_group_3d;
            }
        }
        else {
            return ui_runtime_render_queue_group_zero;
        }
    }
}
