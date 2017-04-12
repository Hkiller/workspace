#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_material.h"
#include "render/runtime/ui_runtime_render_technique.h"
#include "render/runtime/ui_runtime_render_pass.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "render/runtime/ui_runtime_render_program.h"
#include "render/runtime/ui_runtime_render_program_state.h"
#include "render/runtime/ui_runtime_render_program_state_unif.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "plugin_render_ogl_commit_batch_2d.h"
#include "plugin_render_ogl_cache_i.h"
#include "plugin_render_ogl_utils.h"

void plugin_render_ogl_batch_2d_queue_cmd(plugin_render_ogl_module_t module, ui_runtime_render_cmd_t cmd) {
    plugin_render_ogl_batch_2d_t batch_2d = module->m_batch_2d;
    
    if (batch_2d->m_queued_triangle_command_count >= batch_2d->m_queued_triangle_command_capacity) {
        uint32_t new_capacity = batch_2d->m_queued_triangle_command_count < 128 ? 128 : batch_2d->m_queued_triangle_command_count * 2;
        ui_runtime_render_cmd_t * new_buf;

        new_buf = mem_alloc(module->m_alloc, sizeof(ui_runtime_render_cmd_t) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(module->m_em, "plugin_render_ogl_batch_2d_queue_cmd: alloc buf fail, capacity=%d", new_capacity);
            return;
        }

        if (batch_2d->m_queued_triangle_commands) {
            assert(batch_2d->m_queued_triangle_command_count > 0);
            memcpy(new_buf, batch_2d->m_queued_triangle_commands, sizeof(ui_runtime_render_cmd_t) * batch_2d->m_queued_triangle_command_count);
            mem_free(module->m_alloc, batch_2d->m_queued_triangle_commands);
        }

        batch_2d->m_queued_triangle_commands = new_buf;
        batch_2d->m_queued_triangle_command_capacity = new_capacity;
    }

    batch_2d->m_queued_triangle_commands[batch_2d->m_queued_triangle_command_count++] = cmd;
}

static int plugin_render_ogl_batch_2d_fill_vertices_and_indices(
    plugin_render_ogl_module_t module, plugin_render_ogl_batch_2d_t batch_2d, ui_runtime_render_cmd_t cmd)
{
    ui_transform_t mvp = NULL;
    ui_runtime_vertex_v3f_t2f_c4ub_t vertex = batch_2d->m_vertexes + batch_2d->m_vertex_count;
    uint32_t vertex_count = ui_runtime_render_cmd_vertex_count(cmd);
    GLushort * indexes = batch_2d->m_indexes + batch_2d->m_index_count;
    const GLushort * input_indexes = ui_runtime_render_cmd_index_data(cmd);
    uint32_t index_count = ui_runtime_render_cmd_index_count(cmd);
    struct ui_runtime_render_pass_it pass_it;
    ui_runtime_render_pass_t pass;
    uint32_t i;
    
    /*找到一个mvp */
    ui_runtime_render_technique_passes(ui_runtime_render_material_cur_technique(ui_runtime_render_cmd_material(cmd)), &pass_it);
    while((pass = ui_runtime_render_pass_it_next(&pass_it))) {
        ui_runtime_render_program_state_unif_t state_unif = 
            ui_runtime_render_program_state_unif_find_by_buildin(
                ui_runtime_render_pass_program_state(pass), ui_runtime_render_program_unif_matrix_mvp);
        if (state_unif) {
            mvp = &ui_runtime_render_program_state_unif_data(state_unif)->m_data.m_m16;
            break;
        }
    }

    /*构造vertex数据*/
    switch(ui_runtime_render_cmd_vertex_e_type(cmd)) {
    case ui_runtime_render_buff_vertex_v3f_t2f_c4b:
        memcpy(vertex, ui_runtime_render_cmd_vertex_data(cmd), sizeof(vertex[0]) * vertex_count);
        break;
    default:
        CPE_ERROR(
            module->m_em, "plugin_render_ogl_batch_2d_fill_vertices_and_indices: not support index type %s",
            ui_runtime_render_buff_type_to_str(ui_runtime_render_cmd_vertex_e_type(cmd)));
        return -1;
    }

    /*调整mvp, 合并提交数据 */
    if (mvp && ui_transform_cmp(mvp, &UI_TRANSFORM_IDENTITY) != 0) {
        for(i = 0; i < vertex_count; ++i) {
            ui_transform_inline_adj_vector_3(mvp, &vertex[i].m_pos);
        }

        /*设置MVP为统一的无效值 */
        ui_runtime_render_technique_passes(ui_runtime_render_material_cur_technique(ui_runtime_render_cmd_material(cmd)), &pass_it);
        while((pass = ui_runtime_render_pass_it_next(&pass_it))) {
            ui_runtime_render_program_state_unif_t state_unif = 
                ui_runtime_render_program_state_unif_find_by_buildin(
                    ui_runtime_render_pass_program_state(pass), ui_runtime_render_program_unif_matrix_mvp);
            if (state_unif) {
                ui_runtime_render_program_state_unif_data(state_unif)->m_data.m_m16 = UI_TRANSFORM_IDENTITY;
            }
        }
    }

    switch(ui_runtime_render_cmd_index_e_type(cmd)) {
    case ui_runtime_render_buff_index_uint16:
        /*调整索引值 */
        for(i = 0; i < index_count; ++i) {
            indexes[i] = batch_2d->m_vertex_count + input_indexes[i];
        }
        break;
    default:
        CPE_ERROR(
            module->m_em, "plugin_render_ogl_batch_2d_fill_vertices_and_indices: not support element type %s",
            ui_runtime_render_buff_type_to_str(ui_runtime_render_cmd_index_e_type(cmd)));
        return -1;
    }

    batch_2d->m_vertex_count += vertex_count;
    batch_2d->m_index_count += index_count;

    return 0;
}

static plugin_render_ogl_batch_cmd_t
plugin_render_ogl_batch_2d_command_append(plugin_render_ogl_module_t module, plugin_render_ogl_batch_2d_t batch_2d, ui_runtime_render_cmd_t cmd) {
    plugin_render_ogl_batch_cmd_t batch_cmd;
    
    if (batch_2d->m_bached_command_count >= batch_2d->m_bached_command_capacity) {
        uint32_t new_capacity = batch_2d->m_bached_command_capacity < 512 ? 512 : (uint32_t)(batch_2d->m_bached_command_capacity * 1.4);
        void * new_buf = mem_alloc(module->m_alloc, sizeof(struct plugin_render_ogl_batch_cmd) * new_capacity);

        if (new_buf) {
            CPE_ERROR(
                module->m_em, "plugin_render_ogl_batch_2d_init: alloc batch commands fail, capacity=%d",
                new_capacity);
            return NULL;
        }
        
        if (batch_2d->m_bached_commands) {
            assert(batch_2d->m_bached_command_count > 0);
            memcpy(new_buf, batch_2d->m_bached_commands, sizeof(struct plugin_render_ogl_batch_cmd) * batch_2d->m_bached_command_count);
            mem_free(module->m_alloc, batch_2d->m_bached_commands);
        }

        batch_2d->m_bached_commands = new_buf;
        batch_2d->m_bached_command_capacity = new_capacity;
    }

    batch_cmd = &batch_2d->m_bached_commands[batch_2d->m_bached_command_count++];

    if (batch_cmd == batch_2d->m_bached_commands) {
        batch_cmd->m_index_offset = 0;
    }
    else {
        batch_cmd->m_index_offset = (batch_cmd - 1)->m_index_offset + (batch_cmd - 1)->m_index_count;
    }

    batch_cmd->m_cmd = cmd;
    batch_cmd->m_index_count = ui_runtime_render_cmd_index_count(cmd);
    return batch_cmd;
}

static uint8_t plugin_render_ogl_batch_2d_batchable(ui_runtime_render_cmd_t cmd) {
    ui_runtime_render_buff_use_t vertex_buf;

    if (ui_runtime_render_cmd_skip_batch(cmd)) return 0;

    vertex_buf = ui_runtime_render_cmd_vertexs(cmd);
    if (vertex_buf->m_data_source == ui_runtime_render_buff_source_device) return 0;

    return 1;
}

static uint8_t plugin_render_ogl_batch_2d_cmd_compact(ui_runtime_render_cmd_t pre, ui_runtime_render_cmd_t check) {
    ui_runtime_render_technique_t pre_technique;
    struct ui_runtime_render_pass_it pre_pass_it;
    ui_runtime_render_pass_t pre_pass;
    ui_runtime_render_technique_t check_technique;
    struct ui_runtime_render_pass_it check_pass_it;
    ui_runtime_render_pass_t check_pass;
    
    pre_technique = ui_runtime_render_material_cur_technique(ui_runtime_render_cmd_material(pre));
    check_technique = ui_runtime_render_material_cur_technique(ui_runtime_render_cmd_material(check));

    /*渲染阶段数量不匹配 */
    if (ui_runtime_render_technique_pass_count(pre_technique) != ui_runtime_render_technique_pass_count(check_technique)) return 0;
    
    ui_runtime_render_technique_passes(pre_technique, &pre_pass_it);
    ui_runtime_render_technique_passes(check_technique, &check_pass_it);

    while((pre_pass = ui_runtime_render_pass_it_next(&pre_pass_it)), (check_pass = ui_runtime_render_pass_it_next(&check_pass_it))) {
        if (!ui_runtime_render_pass_compatible(pre_pass, check_pass, ui_runtime_render_render_env_compatible_ignore_mvp)) return 0;
    }
    
    return 1;
}

void plugin_render_ogl_batch_2d_commit(plugin_render_ogl_module_t module) {
    plugin_render_ogl_batch_2d_t batch_2d = module->m_batch_2d;
    uint32_t i;
    plugin_render_ogl_batch_cmd_t batch_command = NULL;

    if (batch_2d->m_queued_triangle_command_count == 0) return;
    
    batch_2d->m_vertex_count = 0;
    batch_2d->m_index_count = 0;
    batch_2d->m_bached_command_count = 0;

    for(i = 0; i < batch_2d->m_queued_triangle_command_count; ++i) {
        ui_runtime_render_cmd_t cmd = batch_2d->m_queued_triangle_commands[i];
        uint8_t batchable = plugin_render_ogl_batch_2d_batchable(cmd);

        if (plugin_render_ogl_batch_2d_fill_vertices_and_indices(module, batch_2d, cmd) != 0) continue;

        if (batch_command && batchable && plugin_render_ogl_batch_2d_cmd_compact(batch_command->m_cmd, cmd)) {
            /*可以合并 */
            batch_command->m_index_count += ui_runtime_render_cmd_index_count(cmd);
            batch_command->m_cmd = cmd;
        }
        else {
            /*不可以合并 */
            batch_command = plugin_render_ogl_batch_2d_command_append(module, batch_2d, cmd);
        }

        if (!batchable) batch_command = NULL;
    }

    /*拷贝数据 */
    plugin_render_ogl_active_vao(module, 0);
    
    /*    顶点数据 */
    plugin_render_ogl_bind_buffer(module, plugin_render_ogl_buffer_array, batch_2d->m_vbo[0]);
    plugin_render_ogl_buff_copy_from_mem(
        module, plugin_render_ogl_buffer_array,
        sizeof(batch_2d->m_vertexes[0]) * batch_2d->m_vertex_count, batch_2d->m_vertexes, GL_STATIC_DRAW);

    /*    索引数据 */
    plugin_render_ogl_bind_buffer(module, plugin_render_ogl_buffer_element, batch_2d->m_vbo[1]);
    plugin_render_ogl_buff_copy_from_mem(
        module, plugin_render_ogl_buffer_element,
        sizeof(batch_2d->m_indexes[0]) * batch_2d->m_index_count, batch_2d->m_indexes, GL_STATIC_DRAW);

    /*绘制操作 */
    for (i = 0; i < batch_2d->m_bached_command_count; ++i) {
        struct plugin_render_ogl_batch_cmd * batch_cmd = &batch_2d->m_bached_commands[i];
        ui_runtime_render_cmd_t cmd = batch_cmd->m_cmd;
        ui_runtime_render_material_t material = ui_runtime_render_cmd_material(cmd);
        ui_runtime_render_technique_t technique = ui_runtime_render_material_cur_technique(material);
        struct ui_runtime_render_pass_it pass_it;
        ui_runtime_render_pass_t pass;
        
        ui_runtime_render_technique_passes(technique, &pass_it);
        while((pass = ui_runtime_render_pass_it_next(&pass_it))) {
            if (plugin_render_ogl_bind_pass(module, pass, NULL) != 0) continue;

            glDrawElements(
                GL_TRIANGLES, (GLsizei)batch_2d->m_bached_commands[i].m_index_count, GL_UNSIGNED_SHORT,
                (GLvoid*) (batch_2d->m_bached_commands[i].m_index_offset * sizeof(batch_2d->m_indexes[0])));
            
            DROW_RENDER_OGL_ERROR_DEBUG(module);

            /* uint32_t j; */
            /* for(j = 0; j < batch_cmd->m_index_count; ++j) { */
            /*     uint32_t v_pos = batch_2d->m_indexes[batch_cmd->m_index_offset + j]; */
            /*     ui_runtime_vertex_v3f_t2f_c4ub_t v = batch_2d->m_vertexes + v_pos; */
                
            /*     printf( */
            /*         "draw elements: %d: v=%d, pos=(%f,%f,%f), uv=(%f,%f), color=0x%x\n", */
            /*         j, v_pos, v->m_pos.x, v->m_pos.y, v->m_pos.z, v->m_uv.x, v->m_uv.y, v->m_c); */
            /* } */
            
            module->m_statistics.m_draw_call_count++;
            module->m_statistics.m_vertex_count += batch_cmd->m_index_count;
            module->m_statistics.m_triangles_count += batch_cmd->m_index_count / 3;
        }
    }

    batch_2d->m_queued_triangle_command_count = 0;
    batch_2d->m_vertex_count = 0;
    batch_2d->m_index_count = 0;
}

void plugin_render_ogl_batch_2d_clear(plugin_render_ogl_module_t module) {
    plugin_render_ogl_batch_2d_t batch_2d = module->m_batch_2d;
    
    batch_2d->m_queued_triangle_vertex_count = 0;
    batch_2d->m_queued_triangle_index_count = 0;
    batch_2d->m_queued_triangle_command_count = 0;
}

int plugin_render_ogl_batch_2d_init(plugin_render_ogl_module_t module) {
    plugin_render_ogl_batch_2d_t batch_2d;
    
    batch_2d = mem_alloc(
        module->m_alloc,
        sizeof(struct plugin_render_ogl_batch_2d)
        + sizeof(struct ui_runtime_vertex_v3f_t2f_c4ub) * module->m_capacity_vertex_vbo_size
        + sizeof(GLushort) * module->m_capacity_index_vbo_size);
    if (batch_2d == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_render_ogl_batch_2d_init: alloc fail, vertex_vbo_size=%d, index_vbo_size=%d",
            module->m_capacity_vertex_vbo_size, module->m_capacity_index_vbo_size);
        return -1;
    }

    batch_2d->m_vertex_count = 0;
    batch_2d->m_vertexes = (void*)(batch_2d + 1);
    batch_2d->m_index_count = 0;
    batch_2d->m_indexes = (void*)(batch_2d->m_vertexes + module->m_capacity_vertex_vbo_size);
        
    batch_2d->m_queued_triangle_vertex_count = 0;
    batch_2d->m_queued_triangle_index_count = 0;
    batch_2d->m_queued_triangle_command_count = 0;
    batch_2d->m_queued_triangle_command_capacity = 0;
    batch_2d->m_queued_triangle_commands = NULL;

    batch_2d->m_bached_command_capacity = 512;
    batch_2d->m_bached_commands = mem_alloc(module->m_alloc, sizeof(struct plugin_render_ogl_batch_cmd) * batch_2d->m_bached_command_capacity);
    if (batch_2d->m_bached_commands == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_render_ogl_batch_2d_init: alloc batch commands fail, capacity=%d",
            batch_2d->m_bached_command_capacity);
        mem_free(module->m_alloc, batch_2d);
        return -1;
    }
    batch_2d->m_bached_command_count = 0;

    glGenBuffers(CPE_ARRAY_SIZE(batch_2d->m_vbo), batch_2d->m_vbo);
    
    DROW_RENDER_OGL_ERROR_DEBUG(module);

    module->m_batch_2d = batch_2d;
    return 0;
}

void plugin_render_ogl_batch_2d_fini(plugin_render_ogl_module_t module) {
    plugin_render_ogl_batch_2d_t batch_2d = module->m_batch_2d;
    
    if (batch_2d->m_queued_triangle_commands) {
        mem_free(module->m_alloc, batch_2d->m_queued_triangle_commands);
        batch_2d->m_queued_triangle_commands = NULL;
    }

    if (batch_2d->m_bached_commands) {
        mem_free(module->m_alloc, batch_2d->m_bached_commands);
        batch_2d->m_bached_commands = NULL;
    }

    glDeleteBuffers(2, &batch_2d->m_vbo[0]);
    
    mem_free(module->m_alloc, batch_2d);
    module->m_batch_2d = NULL;
}
