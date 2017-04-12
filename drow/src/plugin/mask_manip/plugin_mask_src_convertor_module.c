#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "plugin/package_manip/plugin_package_manip_src_convertor.h"
#include "plugin/mask/plugin_mask_data.h"
#include "plugin_mask_manip_i.h"
#include "plugin_mask_block_builder_i.h"

static int plugin_mask_manip_src_convertor_from_module(void * ctx, ui_data_src_t source_src, ui_data_src_t to_src, cfg_t args) {
    plugin_mask_manip_t manip = ctx;
    plugin_mask_data_t mask_data;
    ui_data_module_t module;
    struct ui_data_img_block_it img_block_it;
    ui_data_img_block_t img_block;
    const char * str_value;
    plugin_mask_data_format_t format = plugin_mask_data_format_bit;
    ui_cache_pixel_field_t source = ui_cache_pixel_field_a;

    if (ui_data_src_load_state(source_src) != ui_data_src_state_loaded) {
        if (ui_data_src_check_load_with_usings(source_src, manip->m_em) != 0) {
            CPE_ERROR(
                manip->m_em, "img-to-mask: module %s load fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), source_src));
            return -1;
        }
    }

    if ((str_value = cfg_get_string(args, "mask-format", NULL))) {
        if (plugin_mask_data_format_from_str(str_value, &format) != 0) {
            CPE_ERROR(manip->m_em, "img-to-mask: mask-format %s unknown!", str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(args, "field", NULL))) {
        if (ui_cache_pixel_field_from_str(str_value, &source) != 0) {
            CPE_ERROR(manip->m_em, "img-to-mask: source %s unknown!", str_value);
            return -1;
        }
    }

    module = ui_data_src_product(source_src);
    assert(module);

    mask_data = plugin_mask_data_create(manip->m_mask_module, to_src);
    if (mask_data == NULL) {
        CPE_ERROR(manip->m_em, "img-to-mask: create mask scene fail!");
        return -1;
    }
    plugin_mask_data_set_format(mask_data, format);
    ui_data_src_strings_build_begin(to_src);

    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        UI_IMG_BLOCK const * img_block_data = ui_data_img_block_data(img_block);
        plugin_mask_block_builder_t builder;
        
        builder = plugin_mask_block_builder_create(manip, source, 0, 0, img_block_data->src_w, img_block_data->src_h);
        if (builder == NULL) {
            CPE_ERROR(manip->m_em, "img-to-mask: alloc builder fail!");
            continue;
        }

        if (plugin_mask_block_builder_place_img_block(builder, 0, 0, img_block) != 0) {
            plugin_mask_block_builder_free(builder);
            continue;
        }

        if (plugin_mask_block_builder_create_block(builder, to_src, mask_data, ui_data_img_block_name(img_block)) != 0) {
            plugin_mask_block_builder_free(builder);
            continue;
        }

        plugin_mask_block_builder_free(builder);
    }
    
    return 0;
}

int plugin_mask_manip_src_convertor_module_regist(plugin_mask_manip_t module) {
    plugin_package_manip_src_convertor_t src_convertor;

    src_convertor = plugin_package_manip_src_convertor_create(
        module->m_package_manip, "ibk-to-mask",
        ui_data_src_type_module,
        ui_data_src_type_mask,
        plugin_mask_manip_src_convertor_from_module, module);
    if (src_convertor == NULL) {
        CPE_ERROR(module->m_em, "plugin_mask_manip_src_convertor_regist: create convertor fail!");
        return -1;
    }
    
    return 0;
}

void plugin_mask_manip_src_convertor_module_unregist(plugin_mask_manip_t module) {
    plugin_package_manip_src_convertor_t src_convertor;

    src_convertor = plugin_package_manip_src_convertor_find(module->m_package_manip, "ibk-to-mask");
    if (src_convertor) {
        plugin_package_manip_src_convertor_free(src_convertor);
    }
}
