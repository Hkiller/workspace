#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "render/runtime/ui_runtime_render_program_attr.h"
#include "render/runtime/ui_runtime_render_program_unif.h"
#include "plugin_render_s3d_program_i.hpp"
#include "plugin_render_s3d_cache_i.hpp"
#include "plugin_render_s3d_utils.hpp"

int plugin_render_s3d_program_init(void * ctx, ui_runtime_render_program_t i_program) {
    plugin_render_s3d_program_t program;

    program = (plugin_render_s3d_program_t)ui_runtime_render_program_data(i_program);

    program->m_module = (plugin_render_s3d_module_t)ctx;
    new (&program->m_program) flash::display3D::Program3D();
    
    return 0;
}

void plugin_render_s3d_program_fini(void * ctx, ui_runtime_render_program_t i_program, uint8_t is_external_unloaded) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
    plugin_render_s3d_program_t program;
    program = (plugin_render_s3d_program_t)ui_runtime_render_program_data(i_program);

    if (module->m_cache->m_program == program) {
        plugin_render_s3d_use_program(module, NULL);
    }

    if (!is_external_unloaded) {
        CPE_ERROR(module->m_em, "plugin_render_s3d_program_fini: program %s dispose", ui_runtime_render_program_name(i_program));
        try {
            program->m_program->dispose();
        }
        S3D_REPORT_EXCEPTION_1("program %s: dispose: ", ui_runtime_render_program_name(i_program), );
    }

    program->m_program.~Program3D();
}

int plugin_render_s3d_program_unif_init(void * ctx, ui_runtime_render_program_unif_t program_unif) {
    plugin_render_s3d_program_unif_t s3d_unif = (plugin_render_s3d_program_unif_t)ui_runtime_render_program_unif_data(program_unif);
    s3d_unif->m_source = (plugin_render_s3d_program_unif_source)0;
    s3d_unif->m_index = (uint8_t)-1;
    return 0;
}

void plugin_render_s3d_program_unif_fini(void * ctx, ui_runtime_render_program_unif_t program_unif) {
}

int plugin_render_s3d_program_attr_init(void * ctx, ui_runtime_render_program_attr_t program_attr) {
    plugin_render_s3d_program_attr_t s3d_attr = (plugin_render_s3d_program_attr_t)ui_runtime_render_program_attr_data(program_attr);
    s3d_attr->m_index = (uint8_t)-1;
    return 0;
}

void plugin_render_s3d_program_attr_fini(void * ctx, ui_runtime_render_program_attr_t program_attr) {
}
