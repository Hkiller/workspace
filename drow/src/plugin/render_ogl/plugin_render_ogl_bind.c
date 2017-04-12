#include <assert.h>
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
#include "plugin_render_ogl_module_i.h"
#include "plugin_render_ogl_utils.h"
#include "plugin_render_ogl_cache_i.h"
#include "plugin_render_ogl_program_i.h"

int plugin_render_ogl_bind_program_state(
    plugin_render_ogl_module_t module, ui_runtime_render_program_state_t program_state, ui_transform_t mvp)
{
    ui_runtime_render_program_t program = ui_runtime_render_program_state_program(program_state);
    plugin_render_ogl_program_t ogl_program = ui_runtime_render_program_data(program);
    struct ui_runtime_render_program_state_attr_it state_attr_it;
    ui_runtime_render_program_state_attr_t state_attr;
    struct ui_runtime_render_program_state_unif_it state_unif_it;
    ui_runtime_render_program_state_unif_t state_unif;
    
    if (ogl_program->m_program_id == 0) {
        if (plugin_render_ogl_program_link(program, ogl_program) != 0) return -1;
    }

    plugin_render_ogl_use_program(module, ogl_program->m_program_id);
    plugin_render_ogl_enable_program_attrs(module, ogl_program->m_attr_mask);

    ui_runtime_render_program_state_attrs(program_state, &state_attr_it);
    while((state_attr = ui_runtime_render_program_state_attr_it_next(&state_attr_it))) {
        ui_runtime_render_program_attr_t attr = ui_runtime_render_program_state_attr_attr(state_attr);
        ui_runtime_render_program_state_attr_data_t attr_data = ui_runtime_render_program_state_attr_data(state_attr);
        plugin_render_ogl_program_attr_t ogl_attr = ui_runtime_render_program_attr_data(attr);
        GLenum gl_type;

        switch(attr_data->m_element_type) {
        case CPE_DR_TYPE_FLOAT:
            gl_type = GL_FLOAT;
            break;
        case CPE_DR_TYPE_UINT8:
            gl_type = GL_UNSIGNED_BYTE;
            break;
        default:
            CPE_ERROR(
                module->m_em, "plugin_render_ogl_bind_program_state: program %s type %s not support!",
                ui_runtime_render_program_name(program), dr_type_name(attr_data->m_element_type));
            return -1;
        }
        
        glVertexAttribPointer(
            ogl_attr->m_location,
            attr_data->m_element_count, gl_type, GL_FALSE, attr_data->m_stride, (GLvoid*) (ptr_int_t)attr_data->m_start_pos);

        /* CPE_ERROR( */
        /*     module->m_em, */
        /*     "bind attr %s at %d, strip=%d, start=%d, count=%d, type=%s", */
        /*     ui_runtime_render_program_attr_id_str(attr), ogl_attr->m_location, */
        /*     attr_data->m_stride, attr_data->m_start_pos, attr_data->m_element_count, dr_type_name(attr_data->m_element_type)); */
        
        DROW_RENDER_OGL_ERROR_DEBUG(module);
    }

    ui_runtime_render_program_state_unifs(program_state, &state_unif_it);
    while((state_unif = ui_runtime_render_program_state_unif_it_next(&state_unif_it))) {
        ui_runtime_render_program_unif_t unif = ui_runtime_render_program_state_unif_unif(state_unif);
        ui_runtime_render_program_state_unif_data_t unif_data = ui_runtime_render_program_state_unif_data(state_unif);
        plugin_render_ogl_program_unif_t ogl_unif;
        
        ogl_unif = ui_runtime_render_program_unif_data(unif);

        switch(unif_data->m_type) {
        case ui_runtime_render_program_unif_f:
            glUniform1f(ogl_unif->m_location, (GLfloat)unif_data->m_data.m_f);
            break;
        case ui_runtime_render_program_unif_i:
            glUniform1i(ogl_unif->m_location, (GLint)unif_data->m_data.m_i);
            break;
        case ui_runtime_render_program_unif_v2:
            glUniform2fv(ogl_unif->m_location, 1, (GLfloat*)unif_data->m_data.m_v2.value);
            break;
        case ui_runtime_render_program_unif_v3:
            glUniform3fv(ogl_unif->m_location, 1, (GLfloat*)unif_data->m_data.m_v3.value);
            break;
        case ui_runtime_render_program_unif_v4:
            glUniform4fv(ogl_unif->m_location, 1, (GLfloat*)unif_data->m_data.m_v4.value);
            break;
        case ui_runtime_render_program_unif_m16:
            glUniformMatrix4fv(ogl_unif->m_location, 1, 0, (GLfloat*)ui_transform_calc_matrix_4x4(&unif_data->m_data.m_m16)->m);
            break;
        case ui_runtime_render_program_unif_texture:
            if (ui_cache_res_load_state(unif_data->m_data.m_tex.m_res) != ui_cache_res_loaded) {
                CPE_INFO(
                    module->m_em, "plugin_render_ogl_bind_program_state: texture %p(%s) state is %s not loaded skip",
                    unif_data->m_data.m_tex.m_res, ui_cache_res_path(unif_data->m_data.m_tex.m_res),
                    ui_cache_res_load_state_to_str(ui_cache_res_load_state(unif_data->m_data.m_tex.m_res)));
                return -1;
            }
            
            plugin_render_ogl_bind_texture(
                module,
                unif_data->m_data.m_tex.m_res,
                unif_data->m_data.m_tex.m_min_filter,
                unif_data->m_data.m_tex.m_mag_filter,
                unif_data->m_data.m_tex.m_wrap_s,
                unif_data->m_data.m_tex.m_wrap_t,
                unif_data->m_data.m_tex.m_texture_idx);
            glUniform1i(ogl_unif->m_location, (GLint)unif_data->m_data.m_tex.m_texture_idx);
            break;
        default:
            CPE_ERROR(
                module->m_em, "plugin_render_ogl_bind_program_state: program %s unif %s type %s not support!",
                ui_runtime_render_program_name(program), ui_runtime_render_program_unif_name(unif),
                ui_runtime_render_program_unif_type_to_str(unif_data->m_type));
            return -1;
        }

        /* CPE_ERROR( */
        /*     module->m_em, */
        /*     "program %s unif %s set data type %s", */
        /*     ui_runtime_render_program_name(program), ui_runtime_render_program_unif_name(unif), */
        /*     ui_runtime_render_program_unif_type_to_str(unif_data->m_type)); */
        
        DROW_RENDER_OGL_ERROR_DEBUG(module);
    }
    
    return 0;
}

int plugin_render_ogl_bind_pass(plugin_render_ogl_module_t module, ui_runtime_render_pass_t pass, ui_transform_t mvp) {
    ui_runtime_render_state_t render_state;
    ui_runtime_render_program_state_t program_state;

    program_state = ui_runtime_render_pass_program_state(pass);
    if (plugin_render_ogl_bind_program_state(module, program_state, mvp) != 0) return -1;
    
    render_state = ui_runtime_render_pass_render_state(pass);
    plugin_render_ogl_bind_state(module, ui_runtime_render_pass_render_state(pass));

    DROW_RENDER_OGL_ERROR_DEBUG(module);
    
    return 0;
}

void plugin_render_ogl_bind_state(plugin_render_ogl_module_t module, ui_runtime_render_state_t state) {
    ui_runtime_render_state_data_t state_data;

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_blend))) {
        plugin_render_ogl_set_blend(module, state_data->m_blend_on ? &state_data->m_blend : NULL);
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_scissor))) {
        plugin_render_ogl_set_scissor(module, state_data->m_scissor_on ? &state_data->m_scissor : NULL);
    }

    if ((state_data = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_cull_face))) {
        plugin_render_ogl_set_cull_face(module, state_data->m_cull_face);
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
