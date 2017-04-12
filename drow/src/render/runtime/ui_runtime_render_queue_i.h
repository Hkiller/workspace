#ifndef UI_RUNTIME_RENDER_QUEUE_I_H
#define UI_RUNTIME_RENDER_QUEUE_I_H
#include "render/runtime/ui_runtime_render_queue.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "ui_runtime_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_queue {
    ui_runtime_render_t m_render;
    TAILQ_ENTRY(ui_runtime_render_queue) m_next;
    ui_runtime_render_cmd_list_t m_groups[ui_runtime_render_queue_group_count];
    struct ui_runtime_render_state_data m_saved_state;
};

void ui_runtime_render_queue_real_free(ui_runtime_render_queue_t queue);

void ui_runtime_render_queue_sort(ui_runtime_render_queue_t queue);

void ui_runtime_render_queue_state_save(ui_runtime_render_queue_t queue);
void ui_runtime_render_queue_state_restore(ui_runtime_render_queue_t queue);    
    
ui_runtime_render_queue_group_t
ui_runtime_render_queue_select_type(float logic_z, uint8_t is_3d, uint8_t is_transparent);

#ifdef __cplusplus
}
#endif

#endif
