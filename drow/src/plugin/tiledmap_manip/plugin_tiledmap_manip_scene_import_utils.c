#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_manip_scene_import_utils_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"

int plugin_tiledmap_manip_import_build_layer(
    plugin_tiledmap_manip_import_ctx_t ctx, ui_ed_obj_t scene_obj,
    const char * name, plugin_tiledmap_manip_import_tile_ref_list_t * tile_refs,
    int32_t adj_x, int32_t adj_y, uint32_t tile_w, uint32_t tile_h)
{
    ui_ed_obj_t tileset_obj = NULL;
    TILEDMAP_LAYER * tileset_obj_data;
    struct ui_ed_obj_it tileset_obj_it;
    plugin_tiledmap_manip_import_tile_ref_t tile_ref;
    TILEDMAP_RECT tileset_rect;
    uint8_t rect_have_data = 0;

    ui_ed_obj_childs(&tileset_obj_it, scene_obj);
    while((tileset_obj = ui_ed_obj_it_next(&tileset_obj_it))) {
        tileset_obj_data = ui_ed_obj_data(tileset_obj);
        if (strcmp(tileset_obj_data->name, name) == 0) break;
    }

    if (tileset_obj == NULL) {
        tileset_obj = ui_ed_obj_new(scene_obj, ui_ed_obj_type_tiledmap_layer);
        if (tileset_obj == NULL) {
            CPE_ERROR(ctx->m_em, "scene import from %s: build layer: create layer %s fail!", ctx->m_proj_path, name);
            return -1;
        }
        tileset_obj_data = ui_ed_obj_data(tileset_obj);
        cpe_str_dup(tileset_obj_data->name, sizeof(tileset_obj_data->name), name);
    }

    assert(tileset_obj);
    assert(tileset_obj_data);

    tileset_obj_data->cell_w = tile_w;
    tileset_obj_data->cell_h = tile_h;

    ui_ed_obj_remove_childs(tileset_obj);

    TAILQ_FOREACH(tile_ref, tile_refs, m_next_for_owner) {
        ui_ed_obj_t tile_obj = NULL;
        TILEDMAP_TILE * tile_obj_data;

        tile_obj = ui_ed_obj_new(tileset_obj, ui_ed_obj_type_tiledmap_tile);
        if (tile_obj == NULL) {
            CPE_ERROR(
                ctx->m_em, "scene import from %s: build layer %s: create tile fail!",
                ctx->m_proj_path, name);
            return -1;
        }
        tile_obj_data = ui_ed_obj_data(tile_obj);

        tile_obj_data->pos.x = adj_x + (int32_t)tile_ref->m_x;
        tile_obj_data->pos.y = adj_y + (int32_t)tile_ref->m_y;
        tile_obj_data->flip_type = tile_ref->m_flip_type;
        tile_obj_data->angle_type = tile_ref->m_angle_type;
        tile_obj_data->ref_type = tiledmap_tile_ref_type_img;
        tile_obj_data->ref_data.img.module_id = ui_ed_src_id(ctx->m_tile_model_src);
        tile_obj_data->ref_data.img.img_block_id = tile_ref->m_tile->m_img_block_id;

        if (rect_have_data) {
            if (tile_obj_data->pos.x < tileset_rect.lt.x) tileset_rect.lt.x = tile_obj_data->pos.x;
            if (tile_obj_data->pos.y < tileset_rect.lt.y) tileset_rect.lt.y = tile_obj_data->pos.y;
            if (tile_obj_data->pos.x + tile_w > tileset_rect.rb.x) tileset_rect.rb.x = tile_obj_data->pos.x + tile_w;
            if (tile_obj_data->pos.y + tile_h > tileset_rect.rb.y) tileset_rect.rb.y = tile_obj_data->pos.y + tile_h;
        }
        else {
            rect_have_data = 1;
            tileset_rect.lt.x = tile_obj_data->pos.x;
            tileset_rect.lt.y = tile_obj_data->pos.y;
            tileset_rect.rb.x = tile_obj_data->pos.x + tile_w;
            tileset_rect.rb.y = tile_obj_data->pos.y + tile_h;
        }
    }

    if (rect_have_data) {
        tileset_obj_data->cell_row_begin = tileset_rect.lt.y / tile_h;
        tileset_obj_data->cell_row_end = (tileset_rect.rb.y / tile_h) - 1;
        tileset_obj_data->cell_col_begin = tileset_rect.lt.x / tile_w;
        tileset_obj_data->cell_col_end = (tileset_rect.rb.x  / tile_w) - 1;
            
    }
    else {
        tileset_obj_data->cell_row_begin = 0;
        tileset_obj_data->cell_row_end = 0;
        tileset_obj_data->cell_col_begin = 0;
        tileset_obj_data->cell_col_end = 0;
    }

    return 0;
}

int plugin_tiledmap_manip_import_load_layer(
    plugin_tiledmap_manip_import_ctx_t ctx,
    ui_cache_pixel_buf_t * pixel_buf, plugin_tiledmap_manip_import_tile_ref_list_t * tile_refs,
    const char * path, const char * name, uint32_t tile_w, uint32_t tile_h)
{
    char full_path[128];
    ui_cache_pixel_level_info_t pixel_level_info;
    uint32_t layer_width, layer_height;
    uint32_t rol_count, col_count;
    uint32_t rol, col;
    plugin_tiledmap_manip_import_tile_t tile;
    plugin_tiledmap_manip_import_tile_ref_t tile_ref;
    struct ui_cache_pixel_buf_rect rect;

    assert(*pixel_buf == NULL);
    *pixel_buf = ui_cache_pixel_buf_create(ctx->m_cache_mgr);
    if (*pixel_buf == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load layer %s: create pixel_buf fail!",
            ctx->m_proj_path, name);
        return -1;
    }

    snprintf(full_path, sizeof(full_path), "%s/%s", ctx->m_proj_path, path);
    if (ui_cache_pixel_buf_load_from_file(*pixel_buf, full_path, ctx->m_em, ctx->m_alloc) != 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load layer %s: load from %s fail!",
            ctx->m_proj_path, name, full_path);
        return -1;
    }

    assert(ui_cache_pixel_buf_level_count(*pixel_buf) == 1);

    pixel_level_info = ui_cache_pixel_buf_level_info_at(*pixel_buf, 0);
    assert(pixel_level_info);

    layer_width = ui_cache_pixel_buf_level_width(pixel_level_info);
    if (layer_width % tile_w) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load layer %s: pixel %s width error, width=%d, tile-width=%d!",
            ctx->m_proj_path, name, full_path, layer_width, tile_w);
        return -1;
    }
    col_count = layer_width / tile_w;
    
    layer_height = ui_cache_pixel_buf_level_height(pixel_level_info);
    if (layer_height % tile_h) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load layer %s: pixel %s height error, height=%d, tile-height=%d!",
            ctx->m_proj_path, name, full_path, layer_height, tile_h);
        return -1;
    }
    rol_count = layer_height / tile_h;

    rect.level = 0;
    for(rol = 0; rol < rol_count; ++rol) {
        for(col = 0; col < col_count; ++col) {
            uint8_t is_empty;

            rect.boundary_lt = col * tile_w;
            rect.boundary_tp = rol * tile_h;
            rect.boundary_rt = rect.boundary_lt + tile_w;
            rect.boundary_bm = rect.boundary_tp + tile_h;

            if (ui_cache_pixel_buf_rect_is_alpha_zero(&is_empty, *pixel_buf, &rect) != 0) {
                CPE_ERROR(
                    ctx->m_em, "scene import from %s: load layer %s: pixel %s: check rect is empty fail",
                    ctx->m_proj_path, name, full_path);
                return -1;
            }

            if (is_empty) continue;

            tile = plugin_tiledmap_manip_import_tile_create(ctx, *pixel_buf, &rect);
            if (tile == NULL) {
                CPE_ERROR(
                    ctx->m_em, "scene import from %s: load layer %s: pixel %s: create tile(%d,%d) fail",
                    ctx->m_proj_path, name, full_path, col, rol);
                return -1;
            }

            tile_ref = plugin_tiledmap_manip_import_tile_ref_create(tile_refs, tile, col * tile_w, rol * tile_h);
            if (tile_ref == NULL) {
                CPE_ERROR(
                    ctx->m_em, "scene import from %s: load layer %s: pixel %s: create tile ref (%d,%d) fail",
                    ctx->m_proj_path, name, full_path, col, rol);
                return -1;
            }
        }
    }

    return 0;
}
