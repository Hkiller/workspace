#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "cpe/cfg/cfg.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "plugin_tiledmap_manip_scene_export_layer_i.h"
#include "plugin_tiledmap_manip_scene_export_texture_i.h"

plugin_tiledmap_scene_export_layer_t
plugin_tiledmap_scene_export_layer_create(plugin_tiledmap_scene_export_scene_t scene, cfg_t cfg) {
    plugin_tiledmap_scene_export_ctx_t ctx = scene->m_ctx;
    plugin_tiledmap_scene_export_layer_t layer;
    const char * layer_name;
    const char * png_name;
    ui_rect layer_rect;

    layer = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_scene_export_layer));
    if (layer == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: create layer: alloc fail!", ctx->m_proj_path, scene->m_scene_path);
        return NULL;
    }
    layer->m_scene = scene;

    layer_name = cfg_get_string(cfg, "layer", NULL);
    if (layer_name == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: create layer: layer name not configured!", ctx->m_proj_path, scene->m_scene_path);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }
    cpe_str_dup(layer->m_layer_name, sizeof(layer->m_layer_name), layer_name);
    
    png_name = cfg_get_string(cfg, "input", NULL);
    if (png_name == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: input not configured!",
            ctx->m_proj_path, scene->m_scene_path, layer_name);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }
    cpe_str_dup(layer->m_png_name, sizeof(layer->m_png_name), png_name);

    layer->m_data_layer = plugin_tiledmap_data_layer_find_by_name(scene->m_data_scene, layer_name);
    if (layer->m_data_layer == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: create layer: layer %s not exist!", ctx->m_proj_path, scene->m_scene_path, layer_name);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }

    if (plugin_tiledmap_data_layer_rect(layer->m_data_layer, &layer_rect) != 0) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: create layer: layer %s calc rect fail!",
            ctx->m_proj_path, scene->m_scene_path, layer_name);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }

    if (TAILQ_EMPTY(&scene->m_layers)) {
        scene->m_scene_rect = layer_rect;
    }
    else {
        if (layer_rect.lt.x < scene->m_scene_rect.lt.x) scene->m_scene_rect.lt.x = layer_rect.lt.x;
        if (layer_rect.lt.y < scene->m_scene_rect.lt.y) scene->m_scene_rect.lt.y = layer_rect.lt.y;
        if (layer_rect.rb.x > scene->m_scene_rect.rb.x) scene->m_scene_rect.rb.x = layer_rect.rb.x;
        if (layer_rect.rb.y > scene->m_scene_rect.rb.y) scene->m_scene_rect.rb.y = layer_rect.rb.y;
    }

    TAILQ_INSERT_TAIL(&scene->m_layers, layer, m_next_for_scene);

    return layer;
}

void plugin_tiledmap_scene_export_layer_free(plugin_tiledmap_scene_export_layer_t layer) {
    plugin_tiledmap_scene_export_ctx_t ctx = layer->m_scene->m_ctx;

    TAILQ_REMOVE(&layer->m_scene->m_layers, layer, m_next_for_scene);

    mem_free(ctx->m_alloc, layer);
}

static int plugin_tiledmap_scene_export_layer_one(
    plugin_tiledmap_scene_export_scene_t scene, plugin_tiledmap_data_layer_t layer, ui_cache_pixel_buf_t pixel_buf)
{
    plugin_tiledmap_scene_export_ctx_t ctx = scene->m_ctx;
    TILEDMAP_LAYER const * layer_data = plugin_tiledmap_data_layer_data(layer);
    uint32_t output_width;
    uint32_t output_height;
    float pos_adj_x;
    float pos_adj_y;
    struct plugin_tiledmap_data_tile_it tile_it;
    plugin_tiledmap_data_tile_t tile;
    ui_data_src_t src_cache = NULL;
    ui_data_module_t module = NULL;
    ui_cache_pixel_buf_t module_pixel_buf = NULL;

    switch(scene->m_scene_export_position) {
    case plugin_tiledmap_manip_export_bottom_left:
        pos_adj_x = 0.0f;
        pos_adj_y = - scene->m_scene_rect.lt.y;
        output_height = scene->m_scene_rect.lt.y < 0.0f ? - scene->m_scene_rect.lt.y : 0.0f;
        output_width = scene->m_scene_rect.rb.x > 0.0f ? scene->m_scene_rect.rb.x : 0.0f;
        break;
    case plugin_tiledmap_manip_export_bottom_right:
        pos_adj_x = - scene->m_scene_rect.lt.x;
        pos_adj_y = - scene->m_scene_rect.lt.y;
        output_height = scene->m_scene_rect.lt.y < 0.0f ? - scene->m_scene_rect.lt.y : 0.0f;
        output_width = scene->m_scene_rect.lt.x < 0.0f ? - scene->m_scene_rect.lt.x : 0.0f;
        break;
    case plugin_tiledmap_manip_export_top_left:
        pos_adj_x = 0.0f;
        pos_adj_y = 0.0f;
        output_height = scene->m_scene_rect.rb.y > 0.0f ? scene->m_scene_rect.rb.y : 0.0f;
        output_width = scene->m_scene_rect.rb.x > 0.0f ? scene->m_scene_rect.rb.x : 0.0f;
        break;
    case plugin_tiledmap_manip_export_top_right:
        pos_adj_x = - scene->m_scene_rect.lt.x;
        pos_adj_y = 0.0f;
        output_height = scene->m_scene_rect.rb.y > 0.0f ? scene->m_scene_rect.rb.y : 0.0f;
        output_width = scene->m_scene_rect.lt.x < 0.0f ? - scene->m_scene_rect.lt.x : 0.0f;
        break;
    default:
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: unknown export position %d!",
            ctx->m_proj_path, scene->m_scene_path, layer_data->name, scene->m_scene_export_position);
        return -1;
    }

    if (output_width <= 0 || output_height <= 0) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: output size (%d-%d) error!",
            ctx->m_proj_path, scene->m_scene_path, layer_data->name, output_width, output_height);
        return -1;
    }

    if (ui_cache_pixel_buf_pixel_buf_create(pixel_buf, output_width, output_height, ui_cache_pf_r8g8b8a8, 1) != 0) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: output size (%d-%d) error!",
            ctx->m_proj_path, scene->m_scene_path, layer_data->name, output_width, output_height);
        return -1;
    }

    plugin_tiledmap_data_layer_tiles(&tile_it, layer);
    while((tile = plugin_tiledmap_data_tile_it_next(&tile_it))) {
        ui_rect tile_rect;
        TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile);
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK const * img_block_data;
        struct ui_cache_pixel_buf_rect src_rect;
        struct ui_cache_pixel_buf_rect to_rect;

        if (tile_data->ref_type != tiledmap_tile_ref_type_img) continue;

        if (plugin_tiledmap_data_tile_rect(tile, &tile_rect, &src_cache) != 0) {
            CPE_ERROR(
                ctx->m_em, "scene export to %s: scene %s: layer %s: calc tile rect fail!", 
                ctx->m_proj_path, scene->m_scene_path, layer_data->name);
            return -1;
        }

        assert(src_cache);

        module = ui_data_src_product(src_cache);
        assert(module);

        img_block = ui_data_img_block_find_by_id(module, tile_data->ref_data.img.img_block_id);
        assert(img_block);

        img_block_data = ui_data_img_block_data(img_block);

        module_pixel_buf = plugin_tiledmap_scene_export_load_pixel_buf(ctx, src_cache);
        if (module_pixel_buf == NULL) {
            CPE_ERROR(
                ctx->m_em, "scene export to %s: scene %s: layer %s: load module "FMT_UINT32_T" pixel buf fail!",
                ctx->m_proj_path, scene->m_scene_path, layer_data->name, ui_data_src_id(src_cache));
            return -1;
        }

        src_rect.level = 0;
        src_rect.boundary_lt = img_block_data->src_x;
        src_rect.boundary_tp = img_block_data->src_y;
        src_rect.boundary_rt = src_rect.boundary_lt + img_block_data->src_w;
        src_rect.boundary_bm = src_rect.boundary_tp + img_block_data->src_h;
            
        to_rect.level = 0;
        to_rect.boundary_lt = pos_adj_x + tile_rect.lt.x;
        to_rect.boundary_tp = pos_adj_y + tile_rect.lt.y;
        to_rect.boundary_rt = pos_adj_x + tile_rect.rb.x;
        to_rect.boundary_bm = pos_adj_y + tile_rect.rb.y;

        if (ui_cache_pixel_buf_rect_op(
                pixel_buf, &to_rect, module_pixel_buf, &src_rect, tile_data->flip_type, tile_data->angle_type, ctx->m_em)
            != 0)
        {
            CPE_ERROR(
                ctx->m_em, "scene export to %s: scene %s: layer %s: op img rect fail!",
                ctx->m_proj_path, scene->m_scene_path, layer_data->name);
            return -1;
        }
    }

    return 0;
}


int plugin_tiledmap_scene_export_layer_do_export(plugin_tiledmap_scene_export_layer_t layer) {
    plugin_tiledmap_scene_export_ctx_t ctx = layer->m_scene->m_ctx;
    ui_cache_pixel_buf_t pixel_buf;
    char path_buf[128];
    char * sep;

    pixel_buf = ui_cache_pixel_buf_create(ctx->m_cache_mgr);
    if (pixel_buf == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: create pixel buf fail!",
            ctx->m_proj_path, layer->m_scene->m_scene_path, layer->m_layer_name);
        return -1;
    }

    if (plugin_tiledmap_scene_export_layer_one(layer->m_scene, layer->m_data_layer, pixel_buf) != 0) {
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }

    snprintf(path_buf, sizeof(path_buf), "%s/%s", ctx->m_proj_path, layer->m_png_name);

    sep = strrchr(path_buf, '/');
    assert(sep);

    *sep = 0;
    if (dir_mk_recursion(path_buf, DIR_DEFAULT_MODE, ctx->m_em, ctx->m_alloc) != 0) {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: create export dir %s fail, errno=%d (%s)!",
            ctx->m_proj_path, layer->m_scene->m_scene_path, layer->m_layer_name, path_buf, errno, strerror(errno));
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }
    
    *sep = '/';

    if (ui_cache_pixel_buf_save_to_file(pixel_buf, path_buf, ctx->m_em, ctx->m_alloc) != 0)  {
        CPE_ERROR(
            ctx->m_em, "scene export to %s: scene %s: layer %s: save to %s fail!",
            ctx->m_proj_path, layer->m_scene->m_scene_path, layer->m_layer_name, path_buf);
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }
    
    ui_cache_pixel_buf_free(pixel_buf);

    return 0;
}

