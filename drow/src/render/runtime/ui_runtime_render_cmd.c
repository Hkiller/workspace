#include <assert.h>
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "ui_runtime_render_cmd_i.h"
#include "ui_runtime_render_state_i.h"
#include "ui_runtime_render_material_i.h"

ui_runtime_render_cmd_t
ui_runtime_render_cmd_create_i(
    ui_runtime_render_queue_t queue, ui_runtime_render_cmd_type_t cmd_type,
    ui_runtime_render_material_t material,
    float logic_z, uint8_t is_3d, uint8_t is_transparent)
{
    ui_runtime_render_t render = queue->m_render;
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_cmd_t cmd;

    assert(render->m_commit_state == ui_runtime_render_commit_state_prepaire);

    cmd = TAILQ_FIRST(&module->m_free_cmds);
    if (cmd) {
        TAILQ_REMOVE(&module->m_free_cmds, cmd, m_next);
    }
    else {
        cmd = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_cmd));
        if (cmd == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_cmd: alloc fail!");
            return NULL;
        }
    }

    cmd->m_queue = queue;
    cmd->m_group_type = ui_runtime_render_queue_select_type(logic_z, is_3d, is_transparent);
    cmd->m_cmd_type = cmd_type;
    cmd->m_skip_batch = 0;
    cmd->m_logic_z = logic_z;
    cmd->m_sub_queue = NULL;
    
    cmd->m_render_state = ui_runtime_render_state_create(render, NULL);
    if (cmd->m_render_state == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_cmd: create render state fail!");
        cmd->m_queue = (ui_runtime_render_queue_t)module;
        TAILQ_INSERT_TAIL(&module->m_free_cmds, cmd, m_next);
        return NULL;
    }
    cmd->m_render_state->m_data = render->m_render_state->m_data;
    
    cmd->m_material = material;
    ui_runtime_render_material_set_parent_render_state(material, cmd->m_render_state);
    
    cmd->m_vertex_buf = ui_runtime_render_buff_inline(NULL, 0, 0);
    cmd->m_index_buf = ui_runtime_render_buff_inline(NULL, 0, 0);
    TAILQ_INSERT_TAIL(&queue->m_groups[cmd->m_group_type], cmd, m_next);
    render->m_cmd_count++;
    
    return cmd;
}

void ui_runtime_render_cmd_free(ui_runtime_render_cmd_t cmd) {
    ui_runtime_render_queue_t queue = cmd->m_queue;
    ui_runtime_module_t module = queue->m_render->m_module;

    ui_runtime_render_material_free(cmd->m_material);
    ui_runtime_render_state_free(cmd->m_render_state);
    
    TAILQ_REMOVE(&queue->m_groups[cmd->m_group_type], cmd, m_next);

    assert(queue->m_render->m_cmd_count > 0);
    queue->m_render->m_cmd_count--;

    if (cmd->m_sub_queue) ui_runtime_render_queue_free(cmd->m_sub_queue);
    
    /*release */
    cmd->m_queue = (ui_runtime_render_queue_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_cmds, cmd, m_next);
}

void ui_runtime_render_cmd_real_free(ui_runtime_render_cmd_t cmd) {
    ui_runtime_module_t module = (ui_runtime_module_t)cmd->m_queue;

    TAILQ_REMOVE(&module->m_free_cmds, cmd, m_next);
    mem_free(module->m_alloc, cmd);
}

ui_runtime_render_cmd_type_t ui_runtime_render_cmd_type(ui_runtime_render_cmd_t cmd) {
    return cmd->m_cmd_type;
}

const char * ui_runtime_render_cmd_type_str(ui_runtime_render_cmd_t cmd) {
    return ui_runtime_render_cmd_type_to_str(cmd->m_cmd_type);
}

ui_runtime_render_queue_group_t ui_runtime_render_cmd_queue_group(ui_runtime_render_cmd_t cmd) {
    return cmd->m_group_type;
}

uint8_t ui_runtime_render_cmd_skip_batch(ui_runtime_render_cmd_t cmd) {
    return cmd->m_skip_batch;
}

void ui_runtime_render_cmd_set_skip_batch(ui_runtime_render_cmd_t cmd, uint8_t skip_batch) {
    cmd->m_skip_batch = skip_batch;
}

ui_runtime_render_buff_use_t ui_runtime_render_cmd_vertexs(ui_runtime_render_cmd_t cmd) {
    return (cmd->m_vertex_buf.m_data_source == ui_runtime_render_buff_source_inline
            && cmd->m_vertex_buf.m_inline.m_buf == NULL)
        ? NULL
        : &cmd->m_vertex_buf;
}

uint32_t ui_runtime_render_cmd_vertex_count(ui_runtime_render_cmd_t cmd) {
    return ui_runtime_render_buff_use_count(&cmd->m_vertex_buf);
}

void const * ui_runtime_render_cmd_vertex_data(ui_runtime_render_cmd_t cmd) {
    return cmd->m_vertex_buf.m_data_source == ui_runtime_render_buff_source_inline
        ? cmd->m_vertex_buf.m_inline.m_buf
        : NULL;
}

ui_runtime_render_buff_type_t ui_runtime_render_cmd_vertex_e_type(ui_runtime_render_cmd_t cmd) {
    return ui_runtime_render_buff_use_type(&cmd->m_vertex_buf);
}

ui_runtime_render_buff_use_t ui_runtime_render_cmd_indexes(ui_runtime_render_cmd_t cmd) {
    return (cmd->m_index_buf.m_data_source == ui_runtime_render_buff_source_inline
            && cmd->m_index_buf.m_inline.m_buf == NULL)
        ? NULL
        : &cmd->m_index_buf;
}

uint32_t ui_runtime_render_cmd_index_count(ui_runtime_render_cmd_t cmd) {
    return ui_runtime_render_buff_use_count(&cmd->m_index_buf);
}

void const * ui_runtime_render_cmd_index_data(ui_runtime_render_cmd_t cmd) {
    return cmd->m_index_buf.m_data_source == ui_runtime_render_buff_source_inline
        ? cmd->m_index_buf.m_inline.m_buf
        : NULL;
}

ui_runtime_render_buff_type_t ui_runtime_render_cmd_index_e_type(ui_runtime_render_cmd_t cmd) {
    return ui_runtime_render_buff_use_type(&cmd->m_index_buf);
}

void ui_runtime_render_cmd_set_3d(ui_runtime_render_cmd_t cmd, uint8_t is_3d, uint8_t is_transparent) {
    ui_runtime_render_queue_group_t new_group_type = ui_runtime_render_queue_select_type(cmd->m_logic_z, is_3d, is_transparent);
    if (new_group_type != cmd->m_group_type) {
        TAILQ_REMOVE(&cmd->m_queue->m_groups[cmd->m_group_type], cmd, m_next);
        cmd->m_group_type = new_group_type;
        TAILQ_INSERT_TAIL(&cmd->m_queue->m_groups[cmd->m_group_type], cmd, m_next);
    }
}

ui_runtime_render_material_t ui_runtime_render_cmd_material(ui_runtime_render_cmd_t cmd) {
    return cmd->m_material;
}

ui_runtime_render_state_t ui_runtime_render_cmd_render_state(ui_runtime_render_cmd_t cmd) {
    return cmd->m_render_state;
}

const char * ui_runtime_render_cmd_type_to_str(ui_runtime_render_cmd_type_t cmd_type) {
    switch(cmd_type) {
    case ui_runtime_render_cmd_triangles:
        return "triangles";
    case ui_runtime_render_cmd_mesh:
        return "mesh";
    case ui_runtime_render_cmd_primitive:
        return "primitive";
    default:
        return "unknown";
    }
}
