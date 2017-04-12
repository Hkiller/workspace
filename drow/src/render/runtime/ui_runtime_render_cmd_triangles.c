#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_material_utils.h"
#include "ui_runtime_render_cmd_i.h"

ui_runtime_render_cmd_t    
ui_runtime_render_cmd_triangles_create(
    ui_runtime_render_t render,
    float logic_z,
    ui_runtime_render_material_t material,
    struct ui_runtime_render_buff_use vertexs,
    struct ui_runtime_render_buff_use indexes,
    ui_transform_t mv, uint8_t is_3d)
{
    ui_runtime_render_cmd_t cmd;
    struct ui_runtime_render_buff_use inline_vertex;
    struct ui_runtime_render_buff_use inline_index;

    if (ui_runtime_render_copy_buf(render, &inline_vertex, &vertexs) != 0) return NULL;
    if (ui_runtime_render_copy_buf(render, &inline_index, &indexes) != 0) return NULL;
    
    if (ui_runtime_render_material_set_unifs_and_attrs(material, mv, ui_runtime_render_buff_use_type(&vertexs)) != 0) return NULL;
    
    cmd = ui_runtime_render_cmd_create_i(
        ui_runtime_render_queue_top(render), ui_runtime_render_cmd_triangles, material, logic_z, is_3d, 0);
    if (cmd == NULL) return cmd;

    cmd->m_vertex_buf = inline_vertex;
    cmd->m_index_buf = inline_index;
    
    return cmd;
}

static uint16_t s_quad_indexes[] = { 0u, 2u, 1u, 0u, 1u, 3u };

ui_runtime_render_cmd_t
ui_runtime_render_cmd_quad_create(
    ui_runtime_render_t render,
    float logic_z,
    ui_runtime_render_material_t material,
    struct ui_runtime_render_buff_use vertexs,
    ui_transform_t mv, uint8_t is_3d)
{
    ui_runtime_render_cmd_t cmd;
    struct ui_runtime_render_buff_use inline_vertex;
    
    if (ui_runtime_render_material_set_unifs_and_attrs(material, mv, ui_runtime_render_buff_use_type(&vertexs)) != 0) return NULL;

    if (ui_runtime_render_copy_buf(render, &inline_vertex, &vertexs) != 0) return NULL;
    
    cmd = ui_runtime_render_cmd_create_i(
        ui_runtime_render_queue_top(render), ui_runtime_render_cmd_triangles, material, logic_z, is_3d, 0);
    if (cmd == NULL) return cmd;

    cmd->m_vertex_buf = inline_vertex;
    cmd->m_index_buf = ui_runtime_render_buff_inline(s_quad_indexes, ui_runtime_render_buff_index_uint16, CPE_ARRAY_SIZE(s_quad_indexes));
    
    return cmd;
}

static uint16_t s_triangle_indexes[] = { 0u, 1u, 2u };

ui_runtime_render_cmd_t
ui_runtime_render_cmd_triangle_create(
    ui_runtime_render_t render,
    float logic_z,
    ui_runtime_render_material_t material,
    struct ui_runtime_render_buff_use vertexs,
    ui_transform_t mv, uint8_t is_3d)
{
    ui_runtime_render_cmd_t cmd;
    struct ui_runtime_render_buff_use inline_vertex;
    
    if (ui_runtime_render_material_set_unifs_and_attrs(material, mv, ui_runtime_render_buff_use_type(&vertexs)) != 0) return NULL;

    if (ui_runtime_render_copy_buf(render, &inline_vertex, &vertexs) != 0) return NULL;
    
    cmd = ui_runtime_render_cmd_create_i(
        ui_runtime_render_queue_top(render), ui_runtime_render_cmd_triangles, material, logic_z, is_3d, 0);
    if (cmd == NULL) return cmd;

    cmd->m_vertex_buf = inline_vertex;
    cmd->m_index_buf = ui_runtime_render_buff_inline(s_triangle_indexes, ui_runtime_render_buff_index_uint16, CPE_ARRAY_SIZE(s_triangle_indexes));
    
    return cmd;
}
