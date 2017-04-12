#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "plugin_render_ogl_module_i.h"
#include "plugin_render_ogl_cache_i.h"

static uint8_t plugin_render_ogl_module_check_for_gl_externsions(const char * externsions, const char * feature) {
    return (externsions && strstr(externsions, feature)) ? 1 : 0;
}

int plugin_render_ogl_module_init_capacity(plugin_render_ogl_module_t module) {
    const char * externsions = (char *)glGetString(GL_EXTENSIONS);
    GLint v;
    
    module->m_capacity_vertex_vbo_size = 65536;
    module->m_capacity_index_vbo_size = module->m_capacity_vertex_vbo_size / 4 * 6;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &v);
    if (v > CPE_TYPE_ARRAY_SIZE(struct plugin_render_ogl_cache, m_textures)) {
        v = CPE_TYPE_ARRAY_SIZE(struct plugin_render_ogl_cache, m_textures);
    }

    module->m_capacity_active_texture_count = v - 1;
    module->m_capacity_install_texture_index = module->m_capacity_active_texture_count;
        
#if DROW_RENDER_USE_VAO
    module->m_capacity_supports_shareable_vao = plugin_render_ogl_module_check_for_gl_externsions(externsions, "vertex_array_object");
#else
    module->m_capacity_supports_shareable_vao = 0;
#endif

    CPE_INFO(
        module->m_em, "Runing: opengl info: render=%s, version=%s, active-texture-count=%d, vao=%s",
        glGetString(GL_RENDERER), glGetString(GL_VERSION),
        module->m_capacity_active_texture_count + 1,
        (module->m_capacity_supports_shareable_vao ? "enable" : "disable"));

    return 0;
}

void plugin_render_ogl_module_fini_capacity(plugin_render_ogl_module_t module) {
}
