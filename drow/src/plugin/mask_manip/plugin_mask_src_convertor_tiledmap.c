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
#include "render/model/ui_data_sprite.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/package_manip/plugin_package_manip_src_convertor.h"
#include "plugin/mask/plugin_mask_data.h"
#include "plugin_mask_manip_i.h"
#include "plugin_mask_block_builder_i.h"

static int plugin_mask_manip_src_convertor_from_tiledmap(void * ctx, ui_data_src_t source_src, ui_data_src_t to_src, cfg_t args) {
    plugin_mask_manip_t manip = ctx;
    plugin_mask_data_t mask_data;
    plugin_tiledmap_data_scene_t tiledmap;
    struct plugin_tiledmap_data_layer_it layer_it;
    plugin_tiledmap_data_layer_t layer;
    const char * str_value;
    plugin_mask_data_format_t format = plugin_mask_data_format_bit;
    ui_cache_pixel_field_t source = ui_cache_pixel_field_a;
    cfg_t layers_cfg = cfg_find_cfg(args, "layers");
    
    if (ui_data_src_load_state(source_src) != ui_data_src_state_loaded) {
        if (ui_data_src_check_load_with_usings(source_src, manip->m_em) != 0) {
            CPE_ERROR(
                manip->m_em, "tiledmap-to-mask: tiledmap %s load fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), source_src));
            return -1;
        }
    }

    if ((str_value = cfg_get_string(args, "mask-format", NULL))) {
        if (plugin_mask_data_format_from_str(str_value, &format) != 0) {
            CPE_ERROR(manip->m_em, "tiledmap-to-mask: mask-format %s unknown!", str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(args, "source", NULL))) {
        if (ui_cache_pixel_field_from_str(str_value, &source) != 0) {
            CPE_ERROR(manip->m_em, "tiledmap-to-mask: source %s unknown!", str_value);
            return -1;
        }
    }
    
    tiledmap = ui_data_src_product(source_src);
    assert(tiledmap);

    mask_data = plugin_mask_data_create(manip->m_mask_module, to_src);
    if (mask_data == NULL) {
        CPE_ERROR(manip->m_em, "tiledmap-to-mask: create mask scene fail!");
        return -1;
    }
    plugin_mask_data_set_format(mask_data, format);
    ui_data_src_strings_build_begin(to_src);
    
    plugin_tiledmap_data_scene_layers(&layer_it, tiledmap);
    while((layer = plugin_tiledmap_data_layer_it_next(&layer_it))) {
        plugin_mask_block_builder_t builder;
        ui_rect bounding;
        struct plugin_tiledmap_data_tile_it tile_it;
        plugin_tiledmap_data_tile_t tile;
        
        if (layers_cfg) {
            struct cfg_it child_it;
            cfg_t layer_name_cfg;
            uint8_t need_process = 0;
            
            cfg_it_init(&child_it, layers_cfg);
            while((layer_name_cfg = cfg_it_next(&child_it))) {
                if (strcmp(cfg_as_string(layer_name_cfg, ""), plugin_tiledmap_data_layer_name(layer)) == 0) {
                    need_process = 1;
                    break;
                }
            }

            if (!need_process) continue;
        }

        if (plugin_tiledmap_data_layer_rect(layer, &bounding) != 0) {
            CPE_ERROR(
                manip->m_em, "tiledmap-to-mask: %s layer %s calc bounding fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), source_src),
                plugin_tiledmap_data_layer_name(layer));
            continue;
        }
        /* printf("xxxxx: process %s.%s, rect=(%f,%f)-(%f,%f)\n", */
        /*        ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), source_src), plugin_tiledmap_data_layer_name(layer), */
        /*        bounding.lt.x, bounding.lt.y, bounding.rb.x, bounding.rb.y); */

        builder = plugin_mask_block_builder_create(
            manip, source,
            bounding.lt.x, bounding.lt.y,
            ui_rect_width(&bounding), ui_rect_height(&bounding));
        if (builder == NULL) {
            CPE_ERROR(manip->m_em, "tiledmap-to-mask: alloc builder fail!");
            continue;
        }

        plugin_tiledmap_data_layer_tiles(&tile_it, layer);
        while((tile = plugin_tiledmap_data_tile_it_next(&tile_it))) {
            TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile);
            ui_data_src_t using_src;
            int place_x = tile_data->pos.x;
            int place_y = tile_data->pos.y;
                
            if (tile_data->ref_type == tiledmap_tile_ref_type_tag) continue;

            using_src = ui_data_src_find_by_id(ui_data_src_mgr(source_src), plugin_tiledmap_data_tile_src_id(tile));
            if (using_src == NULL) {
                CPE_ERROR(
                    manip->m_em, "tiledmap-to-mask: %s layer %s using src %u!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), source_src),
                    plugin_tiledmap_data_layer_name(layer),
                    plugin_tiledmap_data_tile_src_id(tile));
                continue;
            }
                
            if (tile_data->ref_type == tiledmap_tile_ref_type_img) {
                ui_data_img_block_t img_block;
                
                img_block = ui_data_img_block_find_by_id(ui_data_src_product(using_src), tile_data->ref_data.img.img_block_id);
                if (img_block == NULL) {
                    CPE_ERROR(
                        manip->m_em, "tiledmap-to-mask: using src %s no img-block %d!",
                        ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), using_src),
                        tile_data->ref_data.img.img_block_id);
                    continue;
                }

                if (plugin_mask_block_builder_place_img_block(builder, place_x, place_y, img_block) != 0) {
                    plugin_mask_block_builder_free(builder);
                    continue;
                }
            }
            else if (tile_data->ref_type == tiledmap_tile_ref_type_frame) {
                ui_data_frame_t frame;
                
                frame = ui_data_frame_find_by_id(ui_data_src_product(using_src), tile_data->ref_data.frame.frame_id);
                if (frame == NULL) {
                    CPE_ERROR(
                        manip->m_em, "tiledmap-to-mask: using src %s no frame %u!",
                        ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), using_src),
                        tile_data->ref_data.frame.frame_id);
                    continue;
                }
                
                if (plugin_mask_block_builder_place_frame(builder, place_x, place_y, frame) != 0) {
                    plugin_mask_block_builder_free(builder);
                    continue;
                }
            }
        }
        
        if (plugin_mask_block_builder_create_block(builder, to_src, mask_data, plugin_tiledmap_data_layer_name(layer)) != 0) {
            plugin_mask_block_builder_free(builder);
            continue;
        }

        plugin_mask_block_builder_free(builder);
    }
    
    return 0;
}

int plugin_mask_manip_src_convertor_tiledmap_regist(plugin_mask_manip_t manip) {
    plugin_package_manip_src_convertor_t src_convertor;

    src_convertor = plugin_package_manip_src_convertor_create(
        manip->m_package_manip, "tiledmap-to-mask",
        ui_data_src_type_tiledmap_scene,
        ui_data_src_type_mask,
        plugin_mask_manip_src_convertor_from_tiledmap, manip);
    if (src_convertor == NULL) {
        CPE_ERROR(manip->m_em, "plugin_mask_manip_src_convertor_regist: create convertor fail!");
        return -1;
    }
    
    return 0;
}

void plugin_mask_manip_src_convertor_tiledmap_unregist(plugin_mask_manip_t manip) {
    plugin_package_manip_src_convertor_t src_convertor;

    src_convertor = plugin_package_manip_src_convertor_find(manip->m_package_manip, "tiledmap-to-mask");
    if (src_convertor) {
        plugin_package_manip_src_convertor_free(src_convertor);
    }
}
