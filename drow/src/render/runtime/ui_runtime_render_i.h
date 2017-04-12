#ifndef UI_RUNTIME_RENDER_I_H
#define UI_RUNTIME_RENDER_I_H
#include "render/utils/ui_color.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render.h"
#include "ui_runtime_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_runtime_render_commit_state {
    ui_runtime_render_commit_state_clear
    , ui_runtime_render_commit_state_prepaire
    , ui_runtime_render_commit_state_commit
    , ui_runtime_render_commit_state_done
    , ui_runtime_render_commit_state_skip
} ui_runtime_render_commit_state_t;

struct ui_runtime_render_matrix_stack {
    ui_transform * m_buf;
    uint32_t m_size;
    uint32_t m_capacity;
};

struct ui_runtime_render {
    ui_runtime_module_t m_module;
    TAILQ_ENTRY(ui_runtime_render) m_next_for_module;
    uint8_t m_is_depth_test_for_2d;
    struct ui_runtime_render_statistics m_last_commit_statics;

    uint8_t m_inited;
    ui_color m_clear_color;
    ui_vector_2 m_view_size;
    struct ui_runtime_render_matrix_stack m_matrix_stacks[3];

    /*scissor*/
    ui_rect * m_scissor_buf;
    uint32_t m_scissor_size;
    uint32_t m_scissor_capacity;

    ui_runtime_render_camera_list_t m_cameras;
    ui_runtime_render_camera_t m_active_camera;

    uint32_t m_cmd_count;
    uint32_t m_queue_count;
    ui_runtime_render_queue_t m_default_queue;
    ui_runtime_render_queue_list_t m_queues;
    uint32_t m_queue_stack_count;
    uint32_t m_queue_stack_capacity;
    ui_runtime_render_queue_t * m_queue_stack;

    ui_runtime_render_material_list_t m_materials;
    void * m_data_buf;
    uint32_t m_data_buf_capacity;
    uint32_t m_data_buf_wp;

    ui_runtime_render_state_t m_render_state;
    
    ui_runtime_render_program_list_t m_programs;
    ui_runtime_render_program_state_t m_buildin_programs[ui_runtime_render_program_buildin_count];

    ui_runtime_render_worker_t m_worker;
    ui_runtime_render_commit_state_t m_commit_state;
};

void ui_runtime_render_clear_commit(ui_runtime_render_t context);
void ui_runtime_render_do_commit(ui_runtime_render_t context);

#ifdef __cplusplus
}
#endif

#endif
