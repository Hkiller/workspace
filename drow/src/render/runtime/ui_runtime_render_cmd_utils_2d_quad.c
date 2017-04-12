#include <assert.h>
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "render/runtime/ui_runtime_render_cmd_triangles.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_material.h"
#include "render/runtime/ui_runtime_render_material_utils.h"
#include "ui_runtime_render_cmd_i.h"
#include "ui_runtime_render_pass_i.h"
#include "ui_runtime_render_program_state_i.h"
#include "ui_runtime_render_program_state_unif_i.h"

ui_runtime_render_cmd_t
ui_runtime_render_cmd_quad_create_2d_buildin(
    ui_runtime_render_t render, float logic_z,
    ui_transform_t transform,
    ui_runtime_vertex_v3f_t2f_c4ub_t vertexs,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend)
{
    ui_runtime_render_material_t material;
    ui_runtime_render_cmd_t cmd;
    
    material = ui_runtime_render_material_create_from_buildin_program(render, program);
    if (material == NULL) return NULL;

    if (texture) {
        if (ui_runtime_render_material_set_texture_dft(
                material, texture,
                filter,
                filter,
                ui_runtime_render_texture_clamp_to_edge,
                ui_runtime_render_texture_clamp_to_edge,
                0) != 0)
        {
            ui_runtime_render_material_free(material);
            return NULL;;
        }
    }
    
    cmd = ui_runtime_render_cmd_quad_create(
        render,
        logic_z,
        material,
        ui_runtime_render_buff_inline(vertexs, ui_runtime_render_buff_vertex_v3f_t2f_c4b, 4),
        transform, 0);
    if (cmd == NULL) {
        ui_runtime_render_material_free(material);
        return NULL;
    }

    ui_runtime_render_state_set_blend(ui_runtime_render_cmd_render_state(cmd), blend);
    
    return cmd;
}

int ui_runtime_render_cmd_quad_batch_append(
    ui_runtime_render_cmd_t * cmd,
    ui_runtime_render_t render, float logic_z,
    ui_transform_t transform,
    ui_runtime_vertex_v3f_t2f_c4ub_t vertexs,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend)
{
    ui_runtime_render_buff_use_t buf;
    
    if (*cmd) {
        if (!runtime_render_cmd_state_and_texture_compatible(*cmd, logic_z, transform, texture, filter, program, blend)) {
            if (ui_runtime_render_cmd_quad_batch_commit(cmd, render) != 0) return -1;
        }
    }

    if (*cmd == NULL) {
        ui_runtime_render_material_t material;
    
        material = ui_runtime_render_material_create_from_buildin_program(render, program);
        if (material == NULL) return -1;

        if (texture) {
            if (ui_runtime_render_material_set_texture_dft(
                    material, texture,
                    filter,
                    filter,
                    ui_runtime_render_texture_clamp_to_edge,
                    ui_runtime_render_texture_clamp_to_edge,
                    0) != 0)
            {
                ui_runtime_render_material_free(material);
                return -1;
            }
        }

        if (ui_runtime_render_material_set_unifs_and_attrs(material, transform, ui_runtime_render_buff_vertex_v3f_t2f_c4b) != 0) {
            ui_runtime_render_material_free(material);
            return -1;
        }

        *cmd = ui_runtime_render_cmd_create_i(
            ui_runtime_render_queue_top(render), ui_runtime_render_cmd_triangles, material, logic_z, 0, 0);
        if (*cmd == NULL) return -1;

        ui_runtime_render_state_set_blend(ui_runtime_render_cmd_render_state(*cmd), blend);
    }

    buf = &(*cmd)->m_vertex_buf;
    assert(buf->m_data_source == ui_runtime_render_buff_source_inline);
    
    if (buf->m_inline.m_buf == NULL) {
        void * inline_data_buf = ui_runtime_render_alloc_buf(render, ui_runtime_render_buff_vertex_v3f_t2f_c4b, 4);
        if (inline_data_buf == NULL) {
            ui_runtime_render_cmd_free(*cmd);
            *cmd = NULL;
            return -1;
        }

        memcpy(inline_data_buf, vertexs, sizeof(vertexs[0]) * 4);
        
        *buf = ui_runtime_render_buff_inline(inline_data_buf, ui_runtime_render_buff_vertex_v3f_t2f_c4b, 4);
    }
    else {
        void * inline_data_buf = ui_runtime_render_append_buf(render, buf, 4);
        if (inline_data_buf == NULL) return -1;

        memcpy(inline_data_buf, vertexs, sizeof(vertexs[0]) * 4);
    }

    return 0;
}

int ui_runtime_render_cmd_quad_batch_commit(ui_runtime_render_cmd_t * cmd, ui_runtime_render_t render) {
    uint16_t * index_buf;
    uint16_t * wp;
    uint32_t quad_count;
    uint32_t i;
    
    if (*cmd == NULL) return 0;

    /*为quad命令补充index */
    quad_count = ui_runtime_render_cmd_vertex_count(*cmd);
    assert(quad_count % 4 == 0);
    quad_count /= 4;

    index_buf = ui_runtime_render_alloc_buf(render, ui_runtime_render_buff_index_uint16, quad_count * 6);
    if (index_buf == NULL) {
        CPE_ERROR(
            render->m_module->m_em, "ui_runtime_render_cmd_quad_batch_commit: alloc quade buf fail, count=%d", quad_count);
        ui_runtime_render_cmd_free(*cmd);
        *cmd = NULL;
        return -1;
    }

    wp = index_buf;
    for(i = 0; i < quad_count; ++i) {
        uint16_t base = i * 4;
        *wp++ = base + 0;
        *wp++ = base + 1;
        *wp++ = base + 2;
        *wp++ = base + 0;
        *wp++ = base + 3;
        *wp++ = base + 1;
    }

    (*cmd)->m_index_buf = ui_runtime_render_buff_inline(index_buf, ui_runtime_render_buff_index_uint16, quad_count * 6);
    *cmd = NULL;
    
    return 0;
}
    
