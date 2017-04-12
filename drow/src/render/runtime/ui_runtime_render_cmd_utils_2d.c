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

ui_runtime_render_program_buildin_t
ui_runtime_render_second_color_mix_to_program(ui_runtime_render_second_color_mix_t second_color_mix) {
    switch(second_color_mix) {
    case ui_runtime_render_second_color_none:
        return ui_runtime_render_program_buildin_tex;
    case ui_runtime_render_second_color_add:
        return ui_runtime_render_program_buildin_add;
    case ui_runtime_render_second_color_multiply:
        return ui_runtime_render_program_buildin_multiply;
    case ui_runtime_render_second_color_color:
        return ui_runtime_render_program_buildin_color;
    default:
        return ui_runtime_render_program_buildin_tex;
    }
}

uint8_t runtime_render_cmd_state_and_texture_compatible(
    ui_runtime_render_cmd_t cmd,
    float logic_z,
    ui_transform_t transform,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend)
{
    ui_runtime_render_pass_t pass;
    ui_runtime_render_program_state_unif_t unif;
    ui_runtime_render_program_state_unif_data_t unif_texture;

    if (logic_z != cmd->m_logic_z) return 0;

    if (cmd->m_vertex_buf.m_data_source != ui_runtime_render_buff_source_inline
        || cmd->m_index_buf.m_data_source != ui_runtime_render_buff_source_inline) return 0;
    
    if (!ui_runtime_render_state_blend_compatible(cmd->m_render_state, blend)) return 0;

    if (cmd->m_material->m_cur_technique == NULL) return 0;
    if (cmd->m_material->m_cur_technique->m_pass_count != 1) return 0;

    pass = TAILQ_FIRST(&cmd->m_material->m_cur_technique->m_passes);
    assert(pass);

    if (pass->m_program_state->m_program != cmd->m_queue->m_render->m_buildin_programs[program]->m_program) return 0;

    /*检查texture是否兼容 */
    unif_texture = ui_runtime_render_program_state_find_unif_texture_dft(pass->m_program_state, 0);
    if (texture) {
        if (unif_texture == NULL
            || unif_texture->m_type != ui_runtime_render_program_unif_texture
            || unif_texture->m_data.m_tex.m_res != texture
            || unif_texture->m_data.m_tex.m_min_filter != filter
            || unif_texture->m_data.m_tex.m_mag_filter != filter)
            return 0;
    }
    else {
        if (unif_texture != NULL) return 0;
    }

    /*检查mv是否兼容 */
    unif = ui_runtime_render_program_state_unif_find_by_buildin(pass->m_program_state, ui_runtime_render_program_unif_matrix_mv);
    if (transform) {
        if (unif == NULL
            || unif->m_data.m_type != ui_runtime_render_program_unif_m16
            || ui_transform_cmp(&unif->m_data.m_data.m_m16, transform) != 0)
        {
            return 0;
        }
    }
    else {
        if (unif != NULL) return 0;
    }
    
    return 1;
}
