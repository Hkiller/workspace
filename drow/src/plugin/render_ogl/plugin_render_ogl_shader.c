#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "plugin_render_ogl_shader_i.h"

plugin_render_ogl_shader_t
plugin_render_ogl_shader_create_from_text(plugin_render_ogl_module_t module, const char * name, GLuint shader_type, const char * source) {
    plugin_render_ogl_shader_t shader;

    
    shader = mem_alloc(module->m_alloc, sizeof(struct plugin_render_ogl_shader));
    if (shader == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_ogl_shader: create sahder %s: alloc error", name);
        return NULL;
    }

    shader->m_module = module;
    cpe_str_dup(shader->m_name, sizeof(shader->m_name), name);
    shader->m_shader_type = shader_type;
    shader->m_source = source;
    shader->m_ref_count = 0;
    shader->m_shader_id = 0;

    TAILQ_INSERT_TAIL(&module->m_shaders, shader, m_next);

    return shader;
}

void plugin_render_ogl_shader_free(plugin_render_ogl_shader_t  shader) {
    assert(shader->m_ref_count == 0);

    TAILQ_REMOVE(&shader->m_module->m_shaders, shader, m_next);

    mem_free(shader->m_module->m_alloc, shader);
}

const char * plugin_render_ogl_shader_name(plugin_render_ogl_shader_t shader) {
    return shader->m_name;
}

plugin_render_ogl_shader_t
plugin_render_ogl_shader_find_by_name(plugin_render_ogl_module_t module, const char * name) {
    plugin_render_ogl_shader_t shader;

    TAILQ_FOREACH(shader, &module->m_shaders, m_next) {
        if (strcmp(shader->m_name, name) == 0) return shader;
    }

    return NULL;
}

int plugin_render_ogl_shader_compile(plugin_render_ogl_shader_t shader) {
    GLint shader_result = GL_FALSE;

    assert(shader->m_shader_id == 0);
    
    shader->m_shader_id = glCreateShader(shader->m_shader_type);
    
	/* compile */
	glShaderSource(shader->m_shader_id, 1, &shader->m_source, NULL);
	glCompileShader(shader->m_shader_id);

	glGetShaderiv(shader->m_shader_id, GL_COMPILE_STATUS, &shader_result);
	if (shader_result == GL_FALSE) {
        struct mem_buffer buffer;
		GLint infoLength;
        
        mem_buffer_init(&buffer, shader->m_module->m_alloc);
        
		glGetShaderiv(shader->m_shader_id, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength) {
			glGetShaderInfoLog(shader->m_shader_id, infoLength, NULL, mem_buffer_alloc(&buffer, infoLength));
        }
        else {
            mem_buffer_strcat(&buffer, "");
		}

        CPE_ERROR(
            shader->m_module->m_em, "plugin_render_ogl_shader: sahder %s compile error:\n%s",
            shader->m_name, (const char *)mem_buffer_make_continuous(&buffer, 0));

        mem_buffer_clear_data(&buffer);

        goto COMPILE_ERROR;
	}

    return 0;
    
COMPILE_ERROR:
    if (shader->m_shader_id) {
        glDeleteShader(shader->m_shader_id);
        shader->m_shader_id = 0;
    }

    return -1;
}

void plugin_render_ogl_shader_free_all(plugin_render_ogl_module_t module) {
    while(!TAILQ_EMPTY(&module->m_shaders)) {
        plugin_render_ogl_shader_free(TAILQ_FIRST(&module->m_shaders));
    }
}
