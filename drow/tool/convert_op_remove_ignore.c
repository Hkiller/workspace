#include <assert.h>
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_layout.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"
#include "convert_ctx.h"

static int convert_op_package_remove_ignore_layout(convert_ctx_t ctx, ui_data_src_t src, uint8_t * changed);
static int convert_op_package_remove_ignore_tiledmap(convert_ctx_t ctx, ui_data_src_t src, uint8_t * changed);
static int convert_op_package_remove_ignore_scrollmap(convert_ctx_t ctx, ui_data_src_t src, uint8_t * changed);

int convert_op_remove_ignore(convert_ctx_t ctx, ui_data_src_t src) {
    int rv = 0;
    uint8_t changed = 0;

    switch(ui_data_src_type(src)) {
    case ui_data_src_type_layout:
        if (convert_op_package_remove_ignore_layout(ctx, src, &changed) != 0) rv = -1;
        break;
    case ui_data_src_type_tiledmap_scene:
        if (convert_op_package_remove_ignore_tiledmap(ctx, src, &changed) != 0) rv = -1;
        break;
    case ui_data_src_type_scrollmap_scene:
        if (convert_op_package_remove_ignore_scrollmap(ctx, src, &changed) != 0) rv = -1;
        break;
    default:
        break;
    }

    if (changed) {
        ui_data_src_clear_using(src);
        if (ui_data_src_update_using(src) != 0) {
            CPE_ERROR(
                ctx->m_em, "convert_op_package_remove_ignore: src %s rebuild using fail",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
            rv = -1;
        }
    }
    
    return rv;
}

static uint8_t convert_op_package_remove_ignore_is_res_ignore(convert_ctx_t ctx, ui_data_src_t src, UI_CONTROL_OBJECT_URL const * obj_url) {
    ui_data_src_t ref_src;

    if (obj_url->src_id == 0) return 0;
    
    ref_src = ui_data_src_find_by_id(ctx->m_data_mgr, obj_url->src_id);
    if (ref_src == NULL) {
        CPE_ERROR(
            ctx->m_em, "convert_op_package_remove_ignore: src %s use src %d not exist",
            ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), obj_url->src_id);
        return 0;
    }

    return convert_ctx_is_src_ignore(ctx, ref_src);
}

static uint8_t convert_op_package_remove_ignore_control_is_ignore(convert_ctx_t ctx, ui_data_src_t src, ui_data_control_t control) {
    struct ui_data_control_addition_it addition_it;
    ui_data_control_addition_t addition;

    ui_data_control_additions(&addition_it, control);
    while((addition = ui_data_control_addition_it_next(&addition_it))) {
        UI_CONTROL_ADDITION * addition_data = ui_data_control_addition_data(addition);
        if (addition_data->type != ui_control_addition_type_res_ref) continue;
        if (convert_op_package_remove_ignore_is_res_ignore(ctx, src, &addition_data->data.res_ref.frame.res)) return 1;
    }

    return 0;
}

static int convert_op_package_remove_ignore_control(convert_ctx_t ctx, ui_data_src_t src, ui_data_control_t control, uint8_t * changed) {
    struct ui_data_control_it child_it;
    ui_data_control_t child;
    int rv = 0;

    if (convert_op_package_remove_ignore_control_is_ignore(ctx, src, control)) {
        ui_data_control_free(control);
        *changed = 1;
        return 0;
    }

    ui_data_control_childs(&child_it, control);
    while((child = ui_data_control_it_next(&child_it))) {
        if (convert_op_package_remove_ignore_control(ctx, src, child, changed) != 0) rv = -1;
    }
        
    return rv;
}

static int convert_op_package_remove_ignore_layout(convert_ctx_t ctx, ui_data_src_t src, uint8_t * changed) {
    ui_data_layout_t layout = ui_data_src_product(src);
    return convert_op_package_remove_ignore_control(ctx, src, ui_data_layout_root(layout), changed);
}

static int convert_op_package_remove_ignore_tiledmap(convert_ctx_t ctx, ui_data_src_t src, uint8_t * changed) {
    plugin_tiledmap_data_scene_t scene = ui_data_src_product(src);
    struct plugin_tiledmap_data_layer_it layer_it;
    plugin_tiledmap_data_layer_t layer;
    int rv = 0;
    
    plugin_tiledmap_data_scene_layers(&layer_it, scene);
    while((layer = plugin_tiledmap_data_layer_it_next(&layer_it))) {
        struct plugin_tiledmap_data_tile_it tile_it;
        plugin_tiledmap_data_tile_t tile;
        
        plugin_tiledmap_data_layer_tiles(&tile_it, layer);
        while((tile = plugin_tiledmap_data_tile_it_next(&tile_it))) {
            TILEDMAP_TILE * tile_data = plugin_tiledmap_data_tile_data(tile);
            uint32_t ref_src_id = plugin_tiledmap_data_tile_src_id(tile);
            ui_data_src_t ref_src;
            
            if (ref_src_id == 0) continue;
    
            ref_src = ui_data_src_find_by_id(ctx->m_data_mgr, ref_src_id);
            if (ref_src == NULL) {
                CPE_ERROR(
                    ctx->m_em, "convert_op_package_remove_ignore: src %s use src %d not exist",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), ref_src_id);
                rv = -1;
                continue;
            }

            if (convert_ctx_is_src_ignore(ctx, ref_src)) {
                struct ui_rect rect;
                if (plugin_tiledmap_data_tile_rect(tile, &rect, &ref_src) != 0) {
                    CPE_ERROR(
                        ctx->m_em, "convert_op_package_remove_ignore: src %s calc tile rect fail",
                        ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
                    rv = -1;
                    continue;
                }
                
                tile_data->ref_type = tiledmap_tile_ref_type_tag;
                tile_data->ref_data.tag.rect.lt.x = rect.lt.x - tile_data->pos.x;
                tile_data->ref_data.tag.rect.lt.y = rect.lt.y - tile_data->pos.y;
                tile_data->ref_data.tag.rect.rb.x = rect.rb.x - tile_data->pos.x;
                tile_data->ref_data.tag.rect.rb.y = rect.rb.y - tile_data->pos.y;
                *changed = 1;
            }
        }
    }

    return 0;
}

static int convert_op_package_remove_ignore_scrollmap(convert_ctx_t ctx, ui_data_src_t src, uint8_t * changed) {
    plugin_scrollmap_data_scene_t scene = ui_data_src_product(src);
    int rv = 0;
    struct plugin_scrollmap_data_tile_it tile_it;
    plugin_scrollmap_data_tile_t tile;
        
    plugin_scrollmap_data_scene_tiles(scene, &tile_it);
    while((tile = plugin_scrollmap_data_tile_it_next(&tile_it))) {
        SCROLLMAP_TILE * tile_data = plugin_scrollmap_data_tile_data(tile);
        ui_data_src_t ref_src;
            
        if (tile_data->src_id == 0) continue;
    
        ref_src = ui_data_src_find_by_id(ctx->m_data_mgr, tile_data->src_id);
        if (ref_src == NULL) {
            CPE_ERROR(
                ctx->m_em, "convert_op_package_remove_ignore: src %s use src %d not exist",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), tile_data->src_id);
            rv = -1;
            continue;
        }

        if (convert_ctx_is_src_ignore(ctx, ref_src)) {
            tile_data->res_type = scrollmap_tile_res_type_tag;
            tile_data->src_id = 0;
            tile_data->res_id = 0;
            *changed = 1;
        }
    }

    return 0;
}
