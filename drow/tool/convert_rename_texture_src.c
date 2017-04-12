#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_string_table_builder.h"
#include "render/model/ui_data_module.h"
#include "plugin/particle/plugin_particle_data.h"
#include "convert_ctx.h"

static int convert_rename_texture_src_one(ui_data_src_t src, convert_ctx_t ctx) {
    if (ui_data_src_type(src) == ui_data_src_type_dir) {
        struct ui_data_src_it child_it;
        ui_data_src_t child;
        ui_data_src_childs(&child_it, src);

        while((child = ui_data_src_it_next(&child_it))) {
            if (convert_rename_texture_src_one(child, ctx) != 0) return -1;
        }

        return 0;
    }
    else if (ui_data_src_type(src) == ui_data_src_type_module) {
        ui_data_module_t module;
        struct ui_data_img_block_it img_block_it;
        ui_data_img_block_t img_block;
        
        module = (ui_data_module_t)ui_data_src_product(src);
        assert(module);

        ui_data_img_block_in_module(&img_block_it, module);
        while((img_block = ui_data_img_block_it_next(&img_block_it))) {
            char * path = (char*)ui_data_img_block_using_texture_path(img_block);
            if (file_name_rename_suffix(path, strlen(path) + 1, "pzd") != 0) {
                CPE_ERROR(
                    ctx->m_em, "rename %s texture name '%s' fail!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), path);
                return -1;
            }
        }
        
        return 0;
    }
    else if (ui_data_src_type(src) == ui_data_src_type_particle) {
        plugin_particle_data_t particle;
        struct plugin_particle_data_emitter_it emitter_it;
        plugin_particle_data_emitter_t emitter;
        ui_string_table_builder_t builder;

        builder = ui_data_src_strings_build_begin(src);
        if (builder == NULL) {
            CPE_ERROR(
                ctx->m_em, "%s begin strings builder fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
            return -1;
        }

        particle = (plugin_particle_data_t)ui_data_src_product(src);
        assert(particle);

        plugin_particle_data_emitters(&emitter_it, particle);

        while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
            UI_PARTICLE_EMITTER * particle_data = plugin_particle_data_emitter_data(emitter);
            char texture[256];
            
            if (particle_data->texture_id == 0) {
                CPE_ERROR(
                    ctx->m_em, "%s mod no texture!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
                return -1;
            }

            cpe_str_dup(texture, sizeof(texture), plugin_particle_data_emitter_msg(emitter, particle_data->texture_id));
            if (texture[0] == '.' || texture[0] == '/') {
                CPE_ERROR(
                    ctx->m_em, "%s mod texture %s format error!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), texture);
                return -1;
            }
            
            if (file_name_rename_suffix(texture, sizeof(texture), "pzd") != 0) {
                CPE_ERROR(
                    ctx->m_em, "rename %s texture name '%s' fail!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), texture);
                return -1;
            }

            ui_string_table_builder_msg_free(builder, particle_data->texture_id);
            particle_data->texture_id = ui_string_table_builder_msg_alloc(builder, texture);
        }

        return 0;
    }
    else {
        return 0;
    }
}

int convert_rename_texture_src(convert_ctx_t ctx) {
    return convert_rename_texture_src_one(ui_data_mgr_src_root(ctx->m_data_mgr), ctx);
}
