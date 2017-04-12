#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_module.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/tiledmap_manip/plugin_tiledmap_basic.h"

int plugin_tiledmap_layers_to_pic(
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    const char * scene_path, uint8_t layer_count, const char * * layers, const char * output, error_monitor_t em)
{
    ui_cache_pixel_buf_t pixel_buf;
    ui_data_src_t scene_src;
    plugin_tiledmap_data_scene_t data_scene;
    ui_rect scene_rect;
    TILEDMAP_PAIR pos_adj;
    struct plugin_tiledmap_data_layer_it layer_it;
    plugin_tiledmap_data_layer_t layer;
    char path_buf[128];
    char * sep;
    int rv = 0;
    
    scene_src = ui_data_src_find_by_path(data_mgr, scene_path, ui_data_src_type_tiledmap_scene);
    if (scene_src == NULL) {
        CPE_ERROR(em, "scene export to %s: scene %s not exist!", output, scene_path);
        return -1;
    }

    if (ui_data_src_check_load_with_usings(scene_src, em) != 0) {
        CPE_ERROR(em, "scene export to %s: scene %s load fail!", output, scene_path);
        return -1;
    }

    data_scene = ui_data_src_product(scene_src);
    assert(data_scene);

    if (plugin_tiledmap_data_scene_rect(data_scene, &scene_rect) != 0) {
        CPE_ERROR(em, "scene export to %s: scene %s calc rect fail", output, scene_path);
        return -1;
    }

    pixel_buf = ui_cache_pixel_buf_create(cache_mgr);
    if (pixel_buf == NULL) {
        CPE_ERROR(em, "scene export to %s: create pixel_buf fail!", output);
        return -1;
    }

    if (ui_cache_pixel_buf_pixel_buf_create(
            pixel_buf,
            scene_rect.rb.x - scene_rect.lt.x,
            scene_rect.rb.y - scene_rect.lt.y,
            ui_cache_pf_r8g8b8a8, 1) != 0)
    {
        CPE_ERROR(em, "scene export to %s: create pixel_buf data buf fail!", output);
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }

    pos_adj.x = - scene_rect.lt.x;
    pos_adj.y = - scene_rect.lt.y;

    /*导出数据 */
    plugin_tiledmap_data_scene_layers(&layer_it, data_scene);
    while((layer = plugin_tiledmap_data_layer_it_next(&layer_it))) {
        uint8_t need_process = 1;
        if (layer_count > 0) {
            TILEDMAP_LAYER const * layer_data;
            uint8_t i;
            
            need_process = 0;
            layer_data = plugin_tiledmap_data_layer_data(layer);
            assert(layer_data);

            for(i = 0; i < layer_count; ++i) {
                if (cpe_str_start_with(layer_data->name, layers[i])) {
                    need_process = 1;
                    break;
                }
            }
        }

        if (need_process) {
            if (plugin_tiledmap_layer_to_pixel_buf(pixel_buf, layer, &pos_adj, em) != 0) {
                rv = -1;
            }
        }
    }
    
    /*存盘 */
    cpe_str_dup(path_buf, sizeof(path_buf), output);

    if ((sep = strrchr(path_buf, '/'))) {
        *sep = 0;
        if (dir_mk_recursion(path_buf, DIR_DEFAULT_MODE, em, NULL) != 0) {
            CPE_ERROR(em, "scene export to %s: create dir %s fail!", output, path_buf);
            ui_cache_pixel_buf_free(pixel_buf);
            return -1;
        }
        *sep = '/';
    }
    
    if (ui_cache_pixel_buf_save_to_file(pixel_buf, path_buf, em, NULL) != 0)  {
        CPE_ERROR(em, "scene export to %s: write file fail!", output);
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }
    
    ui_cache_pixel_buf_free(pixel_buf);
    return rv;
}

int plugin_tiledmap_layer_to_pixel_buf(
    ui_cache_pixel_buf_t pixel_buf, plugin_tiledmap_data_layer_t layer, TILEDMAP_PAIR const * pos_adj, error_monitor_t em)
{
    plugin_tiledmap_data_scene_t scene = plugin_tiledmap_data_layer_scene(layer);
    plugin_tiledmap_module_t tiledmap_module = plugin_tiledmap_data_scene_module(scene);
    ui_data_mgr_t data_mgr = plugin_tiledmap_module_data_mgr(tiledmap_module);
    ui_cache_manager_t cache_mgr = ui_data_mgr_cache_mgr(data_mgr);
    const char * root_path;
    TILEDMAP_LAYER const * layer_data = plugin_tiledmap_data_layer_data(layer);
    struct plugin_tiledmap_data_tile_it tile_it;
    plugin_tiledmap_data_tile_t tile;
    float pos_adj_x = 0.0f;
    float pos_adj_y = 0.0f;
    ui_cache_pixel_buf_t module_pixel_buf = NULL;
    ui_data_src_t module_src = NULL;
    ui_data_module_t module = NULL;
    int rv = 0;

    root_path = ui_data_src_data(ui_data_mgr_src_root(data_mgr));
    
    if (pos_adj) {
        pos_adj_x = pos_adj->x;
        pos_adj_y = pos_adj->y;
    }

    plugin_tiledmap_data_layer_tiles(&tile_it, layer);
    while((tile = plugin_tiledmap_data_tile_it_next(&tile_it))) {
        ui_rect tile_rect;
        TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile);
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK const * img_block_data;
        struct ui_cache_pixel_buf_rect src_rect;
        struct ui_cache_pixel_buf_rect to_rect;
        ui_data_src_t tile_module_src = module_src;
        
        if (tile_data->ref_type != tiledmap_tile_ref_type_img) continue;

        if (plugin_tiledmap_data_tile_rect(tile, &tile_rect, &tile_module_src) != 0) {
            CPE_ERROR(em, "tiledmap layer %s to pic: calc tile rect fail!", layer_data->name);
            return -1;
        }

        /*TODO: Loki*/
        /* if (tile_module_src != module_src) { */
        /*     UI_MODULE const * module_data; */
        /*     char path_buf[256]; */

        /*     module = ui_data_src_product(tile_module_src); */
        /*     assert(module); */

        /*     module_data = ui_data_module_data(module); */
        /*     assert(module_data); */

        /*     if (module_pixel_buf) { */
        /*         ui_cache_pixel_buf_pixel_buf_destory(module_pixel_buf); */
        /*     } */
        /*     else { */
        /*         module_pixel_buf = ui_cache_pixel_buf_create(cache_mgr); */
        /*         if (module_pixel_buf == NULL) { */
        /*             CPE_ERROR(em, "tiledmap layer %s to pic: create module texture buf fail!", layer_data->name); */
        /*             rv = -1; */
        /*             continue; */
        /*         } */
        /*     } */

        /*     if (root_path[0]) { */
        /*         snprintf(path_buf, sizeof(path_buf), "%s/%s", root_path, module_data->use_img); */
        /*     } */
        /*     else { */
        /*         cpe_str_dup(path_buf, sizeof(path_buf), module_data->use_img); */
        /*     } */

        /*     if (ui_cache_pixel_buf_load_from_file(module_pixel_buf, path_buf, em, NULL) != 0) { */
        /*         CPE_ERROR( */
        /*             em, "tiledmap layer %s to pic: module "FMT_UINT32_T" use img load from %s fail!", */
        /*             layer_data->name, ui_data_src_id(tile_module_src), path_buf); */
        /*         rv = -1; */
        /*         continue; */
        /*     } */
        /* } */

        /* assert(module); */
        /* assert(module_pixel_buf); */
        
        /* img_block = ui_data_img_block_find_by_id(module, tile_data->ref_data.img.img_block_id); */
        /* assert(img_block); */

        /* img_block_data = ui_data_img_block_data(img_block); */

        /* src_rect.face = 0; */
        /* src_rect.level = 0; */
        /* src_rect.boundary_lt = img_block_data->src_x; */
        /* src_rect.boundary_tp = img_block_data->src_y; */
        /* src_rect.boundary_rt = src_rect.boundary_lt + img_block_data->src_w; */
        /* src_rect.boundary_bm = src_rect.boundary_tp + img_block_data->src_h; */
            
        /* to_rect.face = 0; */
        /* to_rect.level = 0; */
        /* to_rect.boundary_lt = pos_adj_x + tile_rect.lt.x; */
        /* to_rect.boundary_tp = pos_adj_y + tile_rect.lt.y; */
        /* to_rect.boundary_rt = pos_adj_x + tile_rect.rb.x; */
        /* to_rect.boundary_bm = pos_adj_y + tile_rect.rb.y; */

        /* if (ui_cache_pixel_buf_rect_op( */
        /*         pixel_buf, &to_rect, module_pixel_buf, &src_rect, tile_data->flip_type, tile_data->angle_type, em) */
        /*     != 0) */
        /* { */
        /*     CPE_ERROR(em, "tiledmap layer %s to pic: op img rect fail!", layer_data->name); */
        /*     rv = -1; */
        /*     continue; */
        /* } */
    }

    return rv;
}
