#ifndef UI_RENDER_S3D_PROGRAM_I_H
#define UI_RENDER_S3D_PROGRAM_I_H
#include "render/runtime/ui_runtime_render_program.h"
#include "plugin_render_s3d_module_i.hpp"

struct plugin_render_s3d_program {
    plugin_render_s3d_module_t m_module;
    flash::display3D::Program3D m_program;
};

int plugin_render_s3d_program_init(void * ctx, ui_runtime_render_program_t program);
void plugin_render_s3d_program_fini(void * ctx, ui_runtime_render_program_t program, uint8_t is_external_unloaded);

int plugin_render_s3d_program_bind(plugin_render_s3d_program_t program);
void plugin_render_s3d_program_unbind(plugin_render_s3d_program_t program);

/*unif*/
enum plugin_render_s3d_program_unif_source {
    plugin_render_s3d_program_unif_source_fragment = 1,
    plugin_render_s3d_program_unif_source_vertex = 2
};

struct plugin_render_s3d_program_unif {
    enum plugin_render_s3d_program_unif_source m_source;
    uint8_t m_index;
};

int plugin_render_s3d_program_unif_init(void * ctx, ui_runtime_render_program_unif_t program_unif);
void plugin_render_s3d_program_unif_fini(void * ctx, ui_runtime_render_program_unif_t program_unif);

/*attr*/
struct plugin_render_s3d_program_attr {
    int m_index;
};

int plugin_render_s3d_program_attr_init(void * ctx, ui_runtime_render_program_attr_t program_attr);
void plugin_render_s3d_program_attr_fini(void * ctx, ui_runtime_render_program_attr_t program_attr);

#endif
