#include <assert.h>
#include "cpe/utils/bitarry.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_program_state.h"
#include "render/runtime/ui_runtime_render_program_attr.h"
#include "render/runtime/ui_runtime_render_program_unif.h"
#include "plugin_render_s3d_program_i.hpp"
#include "plugin_render_s3d_cache_i.hpp"
#include "plugin_render_s3d_utils.hpp"

int plugin_render_s3d_render_bind(void * ctx, ui_runtime_render_t render) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;

    try {
        module->m_ctx3d = module->m_s3d->context3D;
        plugin_render_s3d_cache_clear(module);

        CPE_INFO(module->m_em, "%s: bind", plugin_render_s3d_module_name(module));

        //shader
        struct {
            const char * m_name;
            const char * m_vertex;
            const char * m_fragment;
            struct {
                const char * m_name;
                ui_runtime_render_program_unif_type_t m_type;
                enum plugin_render_s3d_program_unif_source m_source;
                uint8_t m_index;
            } m_unifs[4];
            struct {
                ui_runtime_render_program_attr_id_t m_attr_id;
            } m_attrs[4];
            ui_runtime_render_program_buildin_t m_buildin;
        }
        default_shaders[] = {
            { "Tex"
              ,
              /*vertex*/
              "m44 op, va0, vc0\n"
              "mov v0, va1\n" /*copy uv*/
              ,
              /*fragment*/
              "tex ft0, v0, fs0 <2d, linear, nomip>\n"
              "mov oc, ft0\n"
              ,
              { { "matrixMVP", ui_runtime_render_program_unif_m16, plugin_render_s3d_program_unif_source_vertex, 0 },
                { "texture0", ui_runtime_render_program_unif_texture, plugin_render_s3d_program_unif_source_fragment, 0  },
                { NULL, (ui_runtime_render_program_unif_type_t)0, (enum plugin_render_s3d_program_unif_source)0, 0  }
              },
              { { ui_runtime_render_program_attr_position },
                { ui_runtime_render_program_attr_texcoord0 },
                { ui_runtime_render_program_attr_max },
              },
              ui_runtime_render_program_buildin_tex },
            { "Modulate"
              ,
              /*vertex*/
              "m44 op, va0, vc0\n"
              "mov v0, va1\n" /*copy uv*/
              "mov v1, va2\n" /*copy color*/
              ,
              /*fragment*/
              "tex ft0, v0, fs0 <2d, linear, nomip>\n"
              "mul oc, ft0, v1\n"
              ,
              { { "matrixMVP", ui_runtime_render_program_unif_m16, plugin_render_s3d_program_unif_source_vertex, 0 },
                { "texture0", ui_runtime_render_program_unif_texture, plugin_render_s3d_program_unif_source_fragment, 0  },
                { NULL, (ui_runtime_render_program_unif_type_t)0, (enum plugin_render_s3d_program_unif_source)0, 0  }
              },
              { { ui_runtime_render_program_attr_position },
                { ui_runtime_render_program_attr_texcoord0 },
                { ui_runtime_render_program_attr_color },
                { ui_runtime_render_program_attr_max },
              },
              ui_runtime_render_program_buildin_multiply },
            { "Add"
              ,
              /*vertex*/
              "m44 op, va0, vc0\n"
              "mov v0, va1\n" /*copy uv*/
              "mov v1, va2\n" /*copy color*/
              ,
              /*fragment*/
              "tex ft0, v0, fs0 <2d, linear, nomip>\n"
              "add ft1.rgb, ft0.rgb, v1.rgb\n"
              "mul ft1.a, ft0.a, v1.a\n"
              "mov oc, ft1\n"
              ,
              { { "matrixMVP", ui_runtime_render_program_unif_m16, plugin_render_s3d_program_unif_source_vertex, 0},
                { "texture0", ui_runtime_render_program_unif_texture, plugin_render_s3d_program_unif_source_fragment, 0 },
                { NULL, (ui_runtime_render_program_unif_type_t)0, (enum plugin_render_s3d_program_unif_source)0, 0  }
              },
              { { ui_runtime_render_program_attr_position },
                { ui_runtime_render_program_attr_texcoord0 },
                { ui_runtime_render_program_attr_color },
                { ui_runtime_render_program_attr_max },
              },
              ui_runtime_render_program_buildin_add },
            { "Color"
              ,
              /*vertex*/
              "m44 op, va0, vc0\n"
              "mov v0, va1\n" /*copy color*/
              ,
              "mov oc, v0\n"
              ,
              { { "matrixMVP", ui_runtime_render_program_unif_m16, plugin_render_s3d_program_unif_source_vertex, 0 },
                { NULL, (ui_runtime_render_program_unif_type_t)0, (enum plugin_render_s3d_program_unif_source)0, 0  }
              },
              { { ui_runtime_render_program_attr_position },
                { ui_runtime_render_program_attr_color },
                { ui_runtime_render_program_attr_max },
              },
              ui_runtime_render_program_buildin_color },
        };

        for (uint32_t i = 0; i < CPE_ARRAY_SIZE(default_shaders); ++i) {
            flash::utils::ByteArray vasm_code;
            try {
                module->m_assembler->assemble(flash::display3D::Context3DProgramType::VERTEX, default_shaders[i].m_vertex);
                vasm_code = module->m_assembler->agalcode;
            }
            S3D_REPORT_EXCEPTION_1("create shader %s: assemble vertex asm", default_shaders[i].m_name, goto BUILD_ERROR);

            flash::utils::ByteArray fasm_code;
            try {
                module->m_assembler->assemble(flash::display3D::Context3DProgramType::FRAGMENT, default_shaders[i].m_fragment);
                fasm_code = module->m_assembler->agalcode;
            }
            S3D_REPORT_EXCEPTION_1("create shader %s: assemble fragment asm", default_shaders[i].m_name, goto BUILD_ERROR);

            ui_runtime_render_program_t program = ui_runtime_render_program_create(render, default_shaders[i].m_name);
            if (program == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create shader %s: create program fail!",
                    plugin_render_s3d_module_name(module), default_shaders[i].m_name);
                goto BUILD_ERROR;
            }
            plugin_render_s3d_program_t s3d_program = (plugin_render_s3d_program_t )ui_runtime_render_program_data(program);

            try {
                s3d_program->m_program = module->m_ctx3d->createProgram();
                s3d_program->m_program->upload(vasm_code, fasm_code);
            }
            catch(AS3::ui::var e) {
                char *err = AS3::ui::internal::utf8_toString(e);
                CPE_ERROR(
                    module->m_em, "%s: create shader %s: upload: %s",
                    plugin_render_s3d_module_name(module), default_shaders[i].m_name, err);
                free(err);
                goto BUILD_ERROR;
            }

            /*unif*/
            for(uint32_t j = 0; j < CPE_ARRAY_SIZE(default_shaders[i].m_unifs); ++j) {
                const char * unif_name = default_shaders[i].m_unifs[j].m_name;
                if (unif_name == NULL) break;

                ui_runtime_render_program_unif_t unif = ui_runtime_render_program_unif_create(program, unif_name, default_shaders[i].m_unifs[j].m_type);
                if (unif == NULL) {
                    CPE_ERROR(module->m_em, "%s: create shaders: create unif %s fail", plugin_render_s3d_module_name(module), unif_name);
                    goto BUILD_ERROR;
                }

                plugin_render_s3d_program_unif_t s3d_unif = (plugin_render_s3d_program_unif_t)ui_runtime_render_program_unif_data(unif);
                s3d_unif->m_source = default_shaders[i].m_unifs[j].m_source;
                s3d_unif->m_index = default_shaders[i].m_unifs[j].m_index;
            }

            /*attr*/
            for(uint32_t j = 0; j < CPE_ARRAY_SIZE(default_shaders[i].m_attrs); ++j) {
                if (default_shaders[i].m_attrs[j].m_attr_id == ui_runtime_render_program_attr_max) break;
            
                ui_runtime_render_program_attr_t attr = ui_runtime_render_program_attr_create(program, default_shaders[i].m_attrs[j].m_attr_id);
                if (attr == NULL) {
                    CPE_ERROR(
                        module->m_em, "%s: create shaders: create attr %s fail",
                        plugin_render_s3d_module_name(module),
                        ui_runtime_render_program_attr_id_to_str(default_shaders[i].m_attrs[j].m_attr_id));
                    goto BUILD_ERROR;
                }

                plugin_render_s3d_program_attr_t s3d_attr = (plugin_render_s3d_program_attr_t)ui_runtime_render_program_attr_data(attr);
                s3d_attr->m_index = (uint8_t)j;
            }
        
            /*buildin program*/
            ui_runtime_render_program_state_t program_state = ui_runtime_render_program_state_create(render, program);
            if (program_state == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create shaders: create buildin program state fail!",
                    plugin_render_s3d_module_name(module));
                goto BUILD_ERROR;
            }

            if (ui_runtime_render_set_buildin_program_state(render, default_shaders[i].m_buildin, program_state) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: create shaders: bind to blend %d fail!",
                    plugin_render_s3d_module_name(module),
                    default_shaders[i].m_buildin);
                ui_runtime_render_program_state_free(program_state);
                goto BUILD_ERROR;
            }
        }

        return 0;

    BUILD_ERROR:
        return -1;
    }
    S3D_REPORT_EXCEPTION("bind", return -1);
}

void plugin_render_s3d_render_unbind(void * ctx, ui_runtime_render_t render) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
    try {
        // plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
        // plugin_render_s3d_shader_free_all(module);
    }
    S3D_REPORT_EXCEPTION("unbind", return);
}
