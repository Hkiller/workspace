#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "render/runtime/ui_runtime_render_program_attr.h"
#include "render/runtime/ui_runtime_render_program_unif.h"
#include "plugin_render_ogl_program_i.h"
#include "plugin_render_ogl_shader_i.h"
#include "plugin_render_ogl_utils.h"

static int plugin_render_ogl_program_init_attrs(ui_runtime_render_program_t i_program, plugin_render_ogl_program_t program);
static int plugin_render_ogl_program_init_unifs(ui_runtime_render_program_t i_program, plugin_render_ogl_program_t program);

int plugin_render_ogl_program_init(void * ctx, ui_runtime_render_program_t i_program) {
    plugin_render_ogl_program_t program;

    program = ui_runtime_render_program_data(i_program);

    program->m_module = ctx;
    program->m_v_shader = NULL;
    program->m_p_shader = NULL;
    program->m_program_id = 0;
    program->m_attr_mask = 0;
    
    return 0;
}

void plugin_render_ogl_program_fini(void * ctx, ui_runtime_render_program_t i_program, uint8_t is_external_unloaded) {
    plugin_render_ogl_program_t program;

    program = ui_runtime_render_program_data(i_program);

    if (program->m_program_id) {
        glDeleteProgram(program->m_program_id);
        program->m_program_id = 0;
    }
    
    if (program->m_v_shader) {
        assert(program->m_v_shader->m_ref_count > 0);
        program->m_v_shader->m_ref_count--;
    }

    if (program->m_p_shader) {
        assert(program->m_p_shader->m_ref_count > 0);
        program->m_p_shader->m_ref_count--;
    }
}

plugin_render_ogl_shader_t plugin_render_ogl_program_v_shader(plugin_render_ogl_program_t program) {
    return program->m_v_shader;
}

int plugin_render_ogl_program_set_v_shader(plugin_render_ogl_program_t program, plugin_render_ogl_shader_t v_shader) {
    assert(program->m_program_id == 0);
    assert(program->m_v_shader == NULL);
    assert(v_shader);

    program->m_v_shader = v_shader;
    program->m_v_shader->m_ref_count++;
    
    return 0;
}

plugin_render_ogl_shader_t plugin_render_ogl_program_p_shader(plugin_render_ogl_program_t program) {
    return program->m_p_shader;
}

int plugin_render_ogl_program_set_p_shader(plugin_render_ogl_program_t program, plugin_render_ogl_shader_t p_shader) {
    assert(program->m_program_id == 0);
    assert(program->m_p_shader == NULL);
    assert(p_shader);

    program->m_p_shader = p_shader;
    program->m_p_shader->m_ref_count++;

    return 0;
}

int plugin_render_ogl_program_link(ui_runtime_render_program_t i_program, plugin_render_ogl_program_t program) {
    GLint link_result;

    assert(program->m_program_id == 0);
    
    program->m_program_id = glCreateProgram();

    /*pshader */
    if (program->m_p_shader->m_shader_id == 0) {
        if (plugin_render_ogl_shader_compile(program->m_p_shader) != 0) goto LINK_ERROR;
    }
    glAttachShader(program->m_program_id, program->m_p_shader->m_shader_id);
    
    /*vshader */
    if (program->m_v_shader->m_shader_id == 0) {
        if (plugin_render_ogl_shader_compile(program->m_v_shader) != 0) goto LINK_ERROR;
    }
    glAttachShader(program->m_program_id, program->m_v_shader->m_shader_id);
        
    
    glLinkProgram(program->m_program_id);

    glGetProgramiv(program->m_program_id, GL_LINK_STATUS, &link_result);

    if (!link_result) {
        struct mem_buffer buffer;
		GLint infoLength;
        
        mem_buffer_init(&buffer, program->m_module->m_alloc);
        
        glGetProgramiv(program->m_program_id, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength) {
            glGetProgramInfoLog(program->m_program_id, infoLength, NULL, mem_buffer_alloc(&buffer, infoLength));
        }
        else {
            mem_buffer_strcat(&buffer, "");
		}

        CPE_ERROR(
            program->m_module->m_em, "ui_runtime: program %s: link error:\n%s",
            ui_runtime_render_program_name(ui_runtime_render_program_from_data(program)),
            (const char *)mem_buffer_make_continuous(&buffer, 0));

        mem_buffer_clear_data(&buffer);

        goto LINK_ERROR;
    }

    if (plugin_render_ogl_program_init_attrs(i_program, program) != 0) goto LINK_ERROR;
    if (plugin_render_ogl_program_init_unifs(i_program, program) != 0) goto LINK_ERROR;

    DROW_RENDER_OGL_ERROR_DEBUG(program->m_module);
    
    CPE_INFO(
        program->m_module->m_em, "ui_runtime: program %s: link success",
        ui_runtime_render_program_name(ui_runtime_render_program_from_data(program)));
    
    return 0;

LINK_ERROR:
    if (program->m_program_id) {
        glDeleteProgram(program->m_program_id);
        program->m_program_id = 0;
    }

    ui_runtime_render_program_clear_attrs_and_unifs(ui_runtime_render_program_from_data(program));

    return -1;
}

int plugin_render_ogl_program_unif_init(void * ctx, ui_runtime_render_program_unif_t program_unif) {
    plugin_render_ogl_program_unif_t ogl_unif = ui_runtime_render_program_unif_data(program_unif);
    bzero(ogl_unif, sizeof(*ogl_unif));
    return 0;
}

void plugin_render_ogl_program_unif_fini(void * ctx, ui_runtime_render_program_unif_t program_unif) {
    plugin_render_ogl_program_unif_t ogl_unif = ui_runtime_render_program_unif_data(program_unif);
    bzero(ogl_unif, sizeof(*ogl_unif));
}

int plugin_render_ogl_program_attr_init(void * ctx, ui_runtime_render_program_attr_t program_attr) {
    plugin_render_ogl_program_attr_t ogl_attr = ui_runtime_render_program_attr_data(program_attr);
    bzero(ogl_attr, sizeof(*ogl_attr));
    return 0;
}

void plugin_render_ogl_program_attr_fini(void * ctx, ui_runtime_render_program_attr_t program_attr) {
    plugin_render_ogl_program_attr_t ogl_attr = ui_runtime_render_program_attr_data(program_attr);
    bzero(ogl_attr, sizeof(*ogl_attr));
}

static int plugin_render_ogl_program_init_attrs(ui_runtime_render_program_t i_program, plugin_render_ogl_program_t ogl_program) {
    ui_runtime_render_program_t program = ui_runtime_render_program_from_data(ogl_program);
	GLint attr_count = 0;
    GLint i;

	glGetProgramiv(ogl_program->m_program_id, GL_ACTIVE_ATTRIBUTES, &attr_count);
	if (attr_count <= 0) return 0;
    
    for (i = 0; i < attr_count; ++i) {
        char name_buf[64];
        GLsizei name_length	= 0;
        GLint type_size = 0;
        GLenum type = 0;
        ui_runtime_render_program_attr_id_t attr_id;
        ui_runtime_render_program_attr_t attr;
        plugin_render_ogl_program_attr_t ogl_attr;

        glGetActiveAttrib(ogl_program->m_program_id, i, CPE_ARRAY_SIZE(name_buf), &name_length, &type_size, &type, name_buf);

        attr_id = plugin_render_ogl_program_attr_id_from_str(name_buf);
        if (attr_id >= ui_runtime_render_program_attr_max) {
            CPE_ERROR(ogl_program->m_module->m_em, "plugin_render_ogl_program_init_attrs: attr %s unkonwn", name_buf);
            continue;
        }

        attr = ui_runtime_render_program_attr_create(program, attr_id);
        if (attr == NULL) {
            CPE_ERROR(ogl_program->m_module->m_em, "plugin_render_ogl_program_init_attrs: attr %s create fail", name_buf);
            continue;
        }

        ogl_attr = ui_runtime_render_program_attr_data(attr);
        ogl_attr->m_location = glGetAttribLocation(ogl_program->m_program_id, name_buf);

        assert(ogl_attr->m_location < 32);
        cpe_ba_set(&ogl_program->m_attr_mask, ogl_attr->m_location, cpe_ba_true);
    }

    return 0;
}

static int plugin_render_ogl_program_init_unifs(ui_runtime_render_program_t i_program, plugin_render_ogl_program_t ogl_program) {
	GLint unif_count = 0;
    GLint i;
	glGetProgramiv(ogl_program->m_program_id, GL_ACTIVE_UNIFORMS, &unif_count);

    for(i = 0; i < unif_count; ++i) {
        ui_runtime_render_program_unif_t unif;
        plugin_render_ogl_program_unif_t ogl_unif;
        char name_buf[64];
        GLsizei name_len = 0;
        GLenum gl_type;
        GLint type_size;
        ui_runtime_render_program_unif_type_t type;
    
        glGetActiveUniform(ogl_program->m_program_id, i, CPE_ARRAY_SIZE(name_buf), &name_len, &type_size, &gl_type, name_buf);

        switch(gl_type) {
        case GL_FLOAT:
            type = ui_runtime_render_program_unif_f;
            break;
        case GL_INT:
            type = ui_runtime_render_program_unif_i;
            break;
        case GL_FLOAT_VEC2:
            type = ui_runtime_render_program_unif_v2;
            break;
        case GL_FLOAT_VEC3:
            type = ui_runtime_render_program_unif_v3;
            break;
        case GL_FLOAT_VEC4:
            type = ui_runtime_render_program_unif_v4;
            break;
        case GL_FLOAT_MAT4:
            type = ui_runtime_render_program_unif_m16;
            break;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_CUBE:
            type = ui_runtime_render_program_unif_texture;
            break;
        default:
            CPE_ERROR(ogl_program->m_module->m_em, "plugin_render_ogl_program_init_unifs: unif %s not support gl type %d", name_buf, gl_type);
            continue;
        }
        
        unif = ui_runtime_render_program_unif_create(i_program, name_buf, type);
        if (unif == NULL) {
            CPE_ERROR(ogl_program->m_module->m_em, "plugin_render_ogl_program_init_unifs: unif %s create fail", name_buf);
            continue;
        }

        ogl_unif = ui_runtime_render_program_unif_data(unif);
        ogl_unif->m_location = glGetUniformLocation(ogl_program->m_program_id, name_buf);

        if (strcmp(name_buf, "matrixMVP") == 0) {
            ui_runtime_render_program_unif_set_buildin(i_program, ui_runtime_render_program_unif_matrix_mvp, unif);
        }
        else if (strcmp(name_buf, "matrixMV") == 0) {
            ui_runtime_render_program_unif_set_buildin(i_program, ui_runtime_render_program_unif_matrix_mv, unif);
        }
        else if (strcmp(name_buf, "matrixP") == 0) {
            ui_runtime_render_program_unif_set_buildin(i_program, ui_runtime_render_program_unif_matrix_p, unif);
        }
	}

    return 0;
}
