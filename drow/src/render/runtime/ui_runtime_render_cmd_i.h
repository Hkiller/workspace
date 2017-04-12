#ifndef UI_RUNTIME_RENDER_CMD_I_H
#define UI_RUNTIME_RENDER_CMD_I_H
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "ui_runtime_render_queue_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_cmd {
    ui_runtime_render_queue_t m_queue;
    TAILQ_ENTRY(ui_runtime_render_cmd) m_next;

    ui_runtime_render_queue_group_t m_group_type;
    ui_runtime_render_cmd_type_t m_cmd_type;
    uint8_t m_skip_batch;
    float m_logic_z;
    float m_depth;

    ui_runtime_render_state_t m_render_state;
    ui_runtime_render_material_t m_material;
    struct ui_runtime_render_buff_use m_vertex_buf;
    struct ui_runtime_render_buff_use m_index_buf;
    ui_runtime_render_queue_t m_sub_queue;
};

ui_runtime_render_cmd_t
ui_runtime_render_cmd_create_i(
    ui_runtime_render_queue_t queue, ui_runtime_render_cmd_type_t cmd_type,
    ui_runtime_render_material_t material,
    float logic_z, uint8_t is_3d, uint8_t is_transparent);
    
void ui_runtime_render_cmd_real_free(ui_runtime_render_cmd_t cmd);

#ifdef __cplusplus
}
#endif

#endif
