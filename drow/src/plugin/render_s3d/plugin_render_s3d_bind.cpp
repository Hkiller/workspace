#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "render/utils/ui_transform.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "render/runtime/ui_runtime_render_material.h"
#include "render/runtime/ui_runtime_render_technique.h"
#include "render/runtime/ui_runtime_render_pass.h"
#include "render/runtime/ui_runtime_render_program.h"
#include "render/runtime/ui_runtime_render_program_attr.h"
#include "render/runtime/ui_runtime_render_program_unif.h"
#include "render/runtime/ui_runtime_render_program_state.h"
#include "render/runtime/ui_runtime_render_program_state_attr.h"
#include "render/runtime/ui_runtime_render_program_state_unif.h"
#include "plugin_render_s3d_module_i.hpp"
#include "plugin_render_s3d_utils.hpp"
#include "plugin_render_s3d_cache_i.hpp"
#include "plugin_render_s3d_utils.hpp"
#include "plugin_render_s3d_program_i.hpp"

int plugin_render_s3d_bind_program_state(
    plugin_render_s3d_module_t module, flash::display3D::VertexBuffer3D & v_buf,
    ui_runtime_render_program_state_t program_state, ui_transform_t mvp)
{
    ui_runtime_render_program_t program = ui_runtime_render_program_state_program(program_state);
    plugin_render_s3d_program_t s3d_program = (plugin_render_s3d_program_t)ui_runtime_render_program_data(program);
    struct ui_runtime_render_program_state_attr_it state_attr_it;
    ui_runtime_render_program_state_attr_t state_attr;
    struct ui_runtime_render_program_state_unif_it state_unif_it;
    ui_runtime_render_program_state_unif_t state_unif;

    try {
        plugin_render_s3d_use_program(module, s3d_program);
    }
    S3D_REPORT_EXCEPTION_1("use program: %s", ui_runtime_render_program_name(program), return -1);
    
    //CPE_ERROR(module->m_em, "xxxx: use program %s", ui_runtime_render_program_name(program));

    uint32_t using_vertexes = 0;
    ui_runtime_render_program_state_attrs(program_state, &state_attr_it);
    while((state_attr = ui_runtime_render_program_state_attr_it_next(&state_attr_it))) {
        ui_runtime_render_program_attr_t attr = ui_runtime_render_program_state_attr_attr(state_attr);
        ui_runtime_render_program_state_attr_data_t attr_data = ui_runtime_render_program_state_attr_data(state_attr);
        plugin_render_s3d_program_attr_t s3d_attr = (plugin_render_s3d_program_attr_t)ui_runtime_render_program_attr_data(attr);

        String format;
        
        switch(attr_data->m_element_type) {
        case CPE_DR_TYPE_FLOAT:
            if (attr_data->m_element_count == 1) {
                format = flash::display3D::Context3DVertexBufferFormat::FLOAT_1;
            }
            else if (attr_data->m_element_count == 2) {
                format = flash::display3D::Context3DVertexBufferFormat::FLOAT_2;
            }
            else if (attr_data->m_element_count == 3) {
                format = flash::display3D::Context3DVertexBufferFormat::FLOAT_3;
            }
            else if (attr_data->m_element_count == 4) {
                format = flash::display3D::Context3DVertexBufferFormat::FLOAT_4;
            }
            else {
                CPE_ERROR(
                    module->m_em, "plugin_render_s3d_bind_program_state: program %s type %s not support count %d!",
                    ui_runtime_render_program_name(program), dr_type_name(attr_data->m_element_type), attr_data->m_element_count);
                return -1;
            }
            break;
        case CPE_DR_TYPE_UINT8:
            if (attr_data->m_element_count == 4) {
                format = flash::display3D::Context3DVertexBufferFormat::BYTES_4;
            }
            else {
                CPE_ERROR(
                    module->m_em, "plugin_render_s3d_bind_program_state: program %s type %s not support count %d!",
                    ui_runtime_render_program_name(program), dr_type_name(attr_data->m_element_type), attr_data->m_element_count);
                return -1;
            }
            break;
        default:
            CPE_ERROR(
                module->m_em, "plugin_render_s3d_bind_program_state: program %s type %s not support!",
                ui_runtime_render_program_name(program), dr_type_name(attr_data->m_element_type));
            return -1;
        }

        try {
            assert((attr_data->m_start_pos % 4) == 0);
            module->m_ctx3d->setVertexBufferAt(s3d_attr->m_index, v_buf, attr_data->m_start_pos / 4, format);
            cpe_ba_set(&using_vertexes, s3d_attr->m_index, cpe_ba_true);
        }
        S3D_REPORT_EXCEPTION_1("setVertexBufferAt: %d", s3d_attr->m_index, return -1);

        /* CPE_ERROR( */
        /*     module->m_em, */
        /*     "bind attr %s at %d, strip=%d, start=%d, count=%d, type=%s", */
        /*     ui_runtime_render_program_attr_id_str(attr), s3d_attr->m_location, */
        /*     attr_data->m_stride, attr_data->m_start_pos, attr_data->m_element_count, dr_type_name(attr_data->m_element_type)); */
    }
    plugin_render_s3d_unbind_other_vertexes(module, using_vertexes);
    
    uint32_t using_textures = 0;
    ui_runtime_render_program_state_unifs(program_state, &state_unif_it);
    while((state_unif = ui_runtime_render_program_state_unif_it_next(&state_unif_it))) {
        ui_runtime_render_program_unif_t unif = ui_runtime_render_program_state_unif_unif(state_unif);
        ui_runtime_render_program_state_unif_data_t unif_data = ui_runtime_render_program_state_unif_data(state_unif);
        plugin_render_s3d_program_unif_t s3d_unif;
        
        s3d_unif = (plugin_render_s3d_program_unif_t)ui_runtime_render_program_unif_data(unif);

        String scope;
        switch(s3d_unif->m_source) {
        case plugin_render_s3d_program_unif_source_fragment:
            scope = flash::display3D::Context3DProgramType::FRAGMENT;
            break;
        case plugin_render_s3d_program_unif_source_vertex:
            scope = flash::display3D::Context3DProgramType::VERTEX;
            break;
        default:
            CPE_ERROR(
                module->m_em, "plugin_render_s3d_bind_program_state: program %s unif source %d unknown!",
                ui_runtime_render_program_name(program), s3d_unif->m_source);
            continue;
        }

        switch(unif_data->m_type) {
        case ui_runtime_render_program_unif_f:
/*             glUniform1f(s3d_unif->m_location, (GLfloat)unif_data->m_data.m_f); */
            break;
        case ui_runtime_render_program_unif_i:
/*             glUniform1i(s3d_unif->m_location, (GLint)unif_data->m_data.m_i); */
            break;
        case ui_runtime_render_program_unif_v2:
/*             glUniform2fv(s3d_unif->m_location, 1, (GLfloat*)unif_data->m_data.m_v2.value); */
            break;
        case ui_runtime_render_program_unif_v3:
/*             glUniform3fv(s3d_unif->m_location, 1, (GLfloat*)unif_data->m_data.m_v3.value); */
            break;
        case ui_runtime_render_program_unif_v4:
/*             glUniform4fv(s3d_unif->m_location, 1, (GLfloat*)unif_data->m_data.m_v4.value); */
            break;
        case ui_runtime_render_program_unif_m16: {
            try {
                assert(s3d_unif->m_index % 4 == 0);
                ui_matrix_4x4_t c_m = ui_transform_calc_matrix_4x4(&unif_data->m_data.m_m16);
                module->m_ctx3d->setProgramConstantsFromByteArray(
                    scope, s3d_unif->m_index % 4, 4, internal::get_ram(), (unsigned)&c_m->m[0], (void *)&c_m->m[0]);
            }
            S3D_REPORT_EXCEPTION("setProgramConstantsFromMatrix", return -1);
            break;
        }
        case ui_runtime_render_program_unif_texture:
            try {
                if (ui_cache_res_load_state(unif_data->m_data.m_tex.m_res) != ui_cache_res_loaded) {
                    if (module->m_debug) {
                        CPE_INFO(
                            module->m_em, "plugin_render_s3d_bind_program_state: texture %s state is %s, not loaded skip",
                            ui_cache_res_path(unif_data->m_data.m_tex.m_res),
                            ui_cache_res_load_state_to_str(ui_cache_res_load_state(unif_data->m_data.m_tex.m_res)));
                    }
                    return -1;
                }
                
                plugin_render_s3d_bind_texture(
                    module,
                    unif_data->m_data.m_tex.m_res,
                    unif_data->m_data.m_tex.m_min_filter,
                    unif_data->m_data.m_tex.m_mag_filter,
                    unif_data->m_data.m_tex.m_wrap_s,
                    unif_data->m_data.m_tex.m_wrap_t,
                    unif_data->m_data.m_tex.m_texture_idx);
                cpe_ba_set(&using_textures, unif_data->m_data.m_tex.m_texture_idx, cpe_ba_true);
            }
            S3D_REPORT_EXCEPTION_1("bind texture %s", ui_cache_res_path(unif_data->m_data.m_tex.m_res), return -1);
            break;
        default:
            CPE_ERROR(
                module->m_em, "plugin_render_s3d_bind_program_state: program %s unif %s type %s not support!",
                ui_runtime_render_program_name(program), ui_runtime_render_program_unif_name(unif),
                ui_runtime_render_program_unif_type_to_str(unif_data->m_type));
            return -1;
        }

        /* CPE_ERROR( */
        /*     module->m_em, */
        /*     "program %s unif %s set data type %s", */
        /*     ui_runtime_render_program_name(program), ui_runtime_render_program_unif_name(unif), */
        /*     ui_runtime_render_program_unif_type_to_str(unif_data->m_type)); */
    }

    plugin_render_s3d_unbind_other_textures(module, using_textures);
    
    return 0;
}

int plugin_render_s3d_bind_pass(
    plugin_render_s3d_module_t module, flash::display3D::VertexBuffer3D & v_buf,
    ui_runtime_render_pass_t pass, ui_transform_t mvp)
{
    ui_runtime_render_state_t render_state;
    ui_runtime_render_program_state_t program_state;

    program_state = ui_runtime_render_pass_program_state(pass);
    if (plugin_render_s3d_bind_program_state(module, v_buf, program_state, mvp) != 0) return -1;
    
    render_state = ui_runtime_render_pass_render_state(pass);
    plugin_render_s3d_bind_state(module, ui_runtime_render_pass_render_state(pass));

    return 0;
}

void plugin_render_s3d_bind_state(plugin_render_s3d_module_t module, ui_runtime_render_state_t state) {
    ui_runtime_render_state_data_t state_data;

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_blend))) {
        plugin_render_s3d_set_blend(module, state_data->m_blend_on ? &state_data->m_blend : NULL);
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_scissor))) {
        plugin_render_s3d_set_scissor(module, state_data->m_scissor_on ? &state_data->m_scissor : NULL);
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_cull_face))) {
        plugin_render_s3d_set_cull_face(module, state_data->m_cull_face);
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_depth_test))) {
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_depth_write))) {
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_depth_func))) {
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_stencil_test))) {
    }
    
    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_stencil_write))) {
    }
    
    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_stencil_func))) {
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_stencil_op))) {
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_front_face))) {
    }
}
