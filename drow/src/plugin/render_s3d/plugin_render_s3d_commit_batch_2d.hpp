#ifndef UI_RENDER_S3D_BATCH_2D_I_H
#define UI_RENDER_S3D_BATCH_2D_I_H
#include "plugin_render_s3d_module_i.hpp"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_render_s3d_batch_cmd {
    ui_runtime_render_cmd_t m_cmd;
    uint16_t m_index_count;
    uint16_t m_index_offset;
};

struct plugin_render_s3d_batch_2d {
    uint32_t m_vertex_count;
    ui_runtime_vertex_v3f_t2f_c4ub_t m_vertexes;
    uint32_t m_index_count;
    uint16_t * m_indexes;
    
    uint32_t m_queued_triangle_vertex_count;
    uint32_t m_queued_triangle_index_count;
    uint32_t m_queued_triangle_command_count;
    uint32_t m_queued_triangle_command_capacity;
    ui_runtime_render_cmd_t * m_queued_triangle_commands;

    uint32_t m_bached_command_count;
    uint32_t m_bached_command_capacity;
    plugin_render_s3d_batch_cmd_t m_bached_commands;
};

void plugin_render_s3d_batch_2d_queue_cmd(plugin_render_s3d_module_t module, ui_runtime_render_cmd_t cmd);
void plugin_render_s3d_batch_2d_commit(plugin_render_s3d_module_t module);
void plugin_render_s3d_batch_2d_clear(plugin_render_s3d_module_t module);

int plugin_render_s3d_batch_2d_init(plugin_render_s3d_module_t module);
void plugin_render_s3d_batch_2d_fini(plugin_render_s3d_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif 
