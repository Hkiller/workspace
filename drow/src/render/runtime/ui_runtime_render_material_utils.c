#include "render/runtime/ui_runtime_render_material_utils.h"
#include "ui_runtime_render_state_i.h"
#include "ui_runtime_render_program_state_i.h"
#include "ui_runtime_render_material_i.h"
#include "ui_runtime_render_technique_i.h"
#include "ui_runtime_render_pass_i.h"

ui_runtime_render_material_t
ui_runtime_render_material_create_from_program(
    ui_runtime_render_t render,
    ui_runtime_render_program_state_t program_state)
{
    ui_runtime_render_material_t material;
    ui_runtime_render_technique_t technique;
    
    material = ui_runtime_render_material_create(render);
    if (material == NULL) {
        return NULL;
    }

    technique = ui_runtime_render_technique_create_from_program(material, program_state);
    if (technique == NULL) {
        ui_runtime_render_material_free(material);
        return NULL;
    }

    ui_runtime_render_material_set_cur_technique(material, technique);

    return material;
}

ui_runtime_render_technique_t
ui_runtime_render_technique_create_from_program(
    ui_runtime_render_material_t material,
    ui_runtime_render_program_state_t program_state)
{
    ui_runtime_render_technique_t technique;
    ui_runtime_render_pass_t pass;

    technique = ui_runtime_render_technique_create(material);
    if (technique == NULL) {
        return NULL;
    }

    pass = ui_runtime_render_pass_create(technique);
    if (pass == NULL) {
        ui_runtime_render_technique_free(technique);
        return NULL;
    }

    ui_runtime_render_pass_set_program_state(pass, program_state);
    
    return technique;
}

ui_runtime_render_material_t
ui_runtime_render_material_create_from_buildin_program(ui_runtime_render_t render, ui_runtime_render_program_buildin_t buildin_program) {
    ui_runtime_render_program_state_t program_state;
    ui_runtime_render_material_t material;
    ui_runtime_render_pass_t pass;
    
    program_state = ui_runtime_render_program_state_create_by_buildin_program(render, buildin_program);
    if (program_state == NULL) return NULL;

    material = ui_runtime_render_material_create_from_program(render, program_state);
    if (material == NULL) {
        ui_runtime_render_program_state_free(program_state);
        return NULL;
    }

    pass = TAILQ_FIRST(&material->m_cur_technique->m_passes);
    
    return material;
}

int ui_runtime_render_material_set_unifs(ui_runtime_render_material_t material, ui_transform_t mv) {
    ui_runtime_render_technique_t technique;
    int rv = 0;

    TAILQ_FOREACH(technique, &material->m_techniques, m_next) {
        ui_runtime_render_pass_t pass;
        TAILQ_FOREACH(pass, &technique->m_passes, m_next) {
            if (ui_runtime_render_program_state_set_unifs_buildin(pass->m_program_state, mv) != 0) rv = -1;
        }
    }
    
    return rv;
}

int ui_runtime_render_material_set_attrs(ui_runtime_render_material_t material, ui_runtime_render_buff_type_t buff_type) {
    ui_runtime_render_technique_t technique;
    int rv = 0;

    TAILQ_FOREACH(technique, &material->m_techniques, m_next) {
        ui_runtime_render_pass_t pass;
        TAILQ_FOREACH(pass, &technique->m_passes, m_next) {
            if (ui_runtime_render_program_state_set_attrs_by_buf_type(pass->m_program_state, buff_type) != 0) rv = -1;
        }
    }
    
    return rv;
}

int ui_runtime_render_material_set_unifs_and_attrs(ui_runtime_render_material_t material, ui_transform_t mv, ui_runtime_render_buff_type_t buff_type) {
    ui_runtime_render_technique_t technique;
    int rv = 0;

    TAILQ_FOREACH(technique, &material->m_techniques, m_next) {
        ui_runtime_render_pass_t pass;
        TAILQ_FOREACH(pass, &technique->m_passes, m_next) {
            if (ui_runtime_render_program_state_set_unifs_buildin(pass->m_program_state, mv) != 0) rv = -1;
            if (ui_runtime_render_program_state_set_attrs_by_buf_type(pass->m_program_state, buff_type) != 0) rv = -1;
        }
    }
    
    return rv;
}

int ui_runtime_render_material_set_texture_dft(
    ui_runtime_render_material_t material,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t texture_idx)
{
    ui_runtime_render_technique_t technique;
    int rv = 0;

    TAILQ_FOREACH(technique, &material->m_techniques, m_next) {
        ui_runtime_render_pass_t pass;
        TAILQ_FOREACH(pass, &technique->m_passes, m_next) {
            if (ui_runtime_render_program_state_set_unif_texture_dft(
                    pass->m_program_state, res, min_filter, mag_filter, wrap_s, wrap_t, texture_idx)
                != 0) rv = -1;
        }
    }
    
    return rv;
}
