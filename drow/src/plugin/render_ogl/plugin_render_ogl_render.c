#include <assert.h>
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_program_state.h"
#include "plugin_render_ogl_shader_i.h"
#include "plugin_render_ogl_program_i.h"
#include "plugin_render_ogl_shaders.shader"

int plugin_render_ogl_render_bind(void * ctx, ui_runtime_render_t render) {
    plugin_render_ogl_module_t module = ctx;
    uint32_t i;
    plugin_render_ogl_shader_t vshader;
    
	//shader
    struct {
        const char * m_text;
        const char * m_name;
        ui_runtime_render_program_buildin_t m_buildin;
    }
	default_shaders[] = {
		{ DefaultPS_Tex, "Tex", ui_runtime_render_program_buildin_tex },
		{ DefaultPS_Modulate, "Modulate", ui_runtime_render_program_buildin_multiply },
		{ DefaultPS_Add, "Add", ui_runtime_render_program_buildin_add },
		{ DefaultPS_Color, "Color",  ui_runtime_render_program_buildin_color },
        /* { DefaultPS_Vtx, "Vtx" }, */
	};
    
    vshader = plugin_render_ogl_shader_create_from_text(module, "DefaultVS", GL_VERTEX_SHADER, DefaultVS);
    if (vshader == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create shaders: create defalut vertex shader fail!",
            plugin_render_ogl_module_name(module));
        return -1;
    }

	for (i = 0; i < CPE_ARRAY_SIZE(default_shaders); ++i) {
        ui_runtime_render_program_t program;
        plugin_render_ogl_program_t ogl_program;
        ui_runtime_render_program_state_t program_state;
        plugin_render_ogl_shader_t pshader;

        pshader = plugin_render_ogl_shader_create_from_text(module, default_shaders[i].m_name, GL_FRAGMENT_SHADER, default_shaders[i].m_text);
        if (pshader == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create shaders: create defalut fragment shader shader %s fail!",
                plugin_render_ogl_module_name(module), default_shaders[i].m_name);
            goto BUILD_ERROR;
        }

        program = ui_runtime_render_program_create(render, "");
        if (program == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create shaders: create defalut fragment shader shader %s fail!",
                plugin_render_ogl_module_name(module), default_shaders[i].m_name);
            plugin_render_ogl_shader_free(pshader);
            goto BUILD_ERROR;
        }
        ogl_program = ui_runtime_render_program_data(program);

        plugin_render_ogl_program_set_v_shader(ogl_program, vshader);
        plugin_render_ogl_program_set_p_shader(ogl_program, pshader);

        if (plugin_render_ogl_program_link(program, ogl_program) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create shaders: link fail!",
                plugin_render_ogl_module_name(module));
            goto BUILD_ERROR;
        }

        program_state = ui_runtime_render_program_state_create(render, program);
        if (program_state == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create shaders: create buildin program state fail!",
                plugin_render_ogl_module_name(module));
            goto BUILD_ERROR;
        }

        if (ui_runtime_render_set_buildin_program_state(render, default_shaders[i].m_buildin, program_state) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create shaders: bind to blend %d fail!",
                plugin_render_ogl_module_name(module),
                default_shaders[i].m_buildin);
            ui_runtime_render_program_state_free(program_state);            
            goto BUILD_ERROR;
        }
    }

    return 0;

BUILD_ERROR:
    plugin_render_ogl_shader_free_all(module);
    return -1;
}

void plugin_render_ogl_render_unbind(void * ctx, ui_runtime_render_t render) {
    plugin_render_ogl_module_t module = ctx;
    plugin_render_ogl_shader_free_all(module);
}
