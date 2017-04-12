#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_manip_scene_import_merge_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"

static int
plugin_tiledmap_manip_import_merge_one_way(
    plugin_tiledmap_manip_import_merge_ctx_t merge_ctx,
    plugin_tiledmap_manip_import_tile_t tile, uint8_t flip_type, uint8_t angle_type);

int plugin_tiledmap_manip_import_merge(plugin_tiledmap_manip_import_ctx_t ctx) {
    struct plugin_tiledmap_manip_import_merge_ctx merge_ctx;
    plugin_tiledmap_manip_import_tile_t tile, next_tile;

    if (plugin_tiledmap_manip_import_merge_ctx_init(&merge_ctx, ctx) != 0) {
        CPE_ERROR(ctx->m_em, "plugin_tiledmap_manip_import_merge: init merge ctx error!");
        return -1;
    }

    TAILQ_FOREACH(tile, &ctx->m_tiles, m_next_for_ctx) {
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_none, tiledmap_tile_angle_type_none); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_x, tiledmap_tile_angle_type_none); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_y, tiledmap_tile_angle_type_none); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_xy, tiledmap_tile_angle_type_none); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_none, tiledmap_tile_angle_type_90); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_x, tiledmap_tile_angle_type_90); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_y, tiledmap_tile_angle_type_90); */
        /* plugin_tiledmap_manip_import_tile_dump(tile, tiledmap_tile_flip_type_xy, tiledmap_tile_angle_type_90); */

        if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_x, tiledmap_tile_angle_type_none) != 0) return -1;
        if (tile->m_ref_count == 0) continue;

        if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_y, tiledmap_tile_angle_type_none) != 0) return -1;
        if (tile->m_ref_count == 0) continue;

        if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_xy, tiledmap_tile_angle_type_none) != 0) return -1;
        if (tile->m_ref_count == 0) continue;
        
        if ((tile->m_rect.boundary_rt - tile->m_rect.boundary_lt) == (tile->m_rect.boundary_bm - tile->m_rect.boundary_tp)) {
            if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_none, tiledmap_tile_angle_type_90) != 0) return -1;
            if (tile->m_ref_count == 0) continue;

            if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_x, tiledmap_tile_angle_type_90) != 0) return -1;
            if (tile->m_ref_count == 0) continue;

            if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_y, tiledmap_tile_angle_type_90) != 0) return -1;
            if (tile->m_ref_count == 0) continue;

            if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_xy, tiledmap_tile_angle_type_90) != 0) return -1;
            if (tile->m_ref_count == 0) continue;
        }

        /*最终没有放置成功，以原始的状态创建一个tile串 */
        if (plugin_tiledmap_manip_import_merge_one_way(&merge_ctx, tile, tiledmap_tile_flip_type_none, tiledmap_tile_angle_type_none) != 0) return -1;
    }

    plugin_tiledmap_manip_import_merge_ctx_fini(&merge_ctx);

    /*清除不再使用的Tile */
    for(tile = TAILQ_FIRST(&ctx->m_tiles); tile; tile = next_tile) {
        next_tile = TAILQ_NEXT(tile, m_next_for_ctx);

        if (TAILQ_EMPTY(&tile->m_refs)) {
            plugin_tiledmap_manip_import_tile_free(tile);
        }
    }

    return 0;
}

static int
plugin_tiledmap_manip_import_merge_one_way(
    plugin_tiledmap_manip_import_merge_ctx_t merge_ctx,
    plugin_tiledmap_manip_import_tile_t tile, uint8_t flip_type, uint8_t angle_type)
{
    struct cpe_md5_value md5_value;
    plugin_tiledmap_manip_import_merge_tile_t merge_tile;
    uint32_t width = tile->m_rect.boundary_rt - tile->m_rect.boundary_lt;
    uint32_t height = tile->m_rect.boundary_bm - tile->m_rect.boundary_tp;

    if (flip_type || angle_type) {
        ui_cache_pixel_level_info_t level_info;
        struct ui_cache_pixel_buf_rect tmp_rect;

        if (ui_cache_pixel_buf_level_count(merge_ctx->m_tmp_pixel_buf) == 0) {
            if (ui_cache_pixel_buf_pixel_buf_create(merge_ctx->m_tmp_pixel_buf, width, height, ui_cache_pf_r8g8b8a8, 1) != 0) {
                CPE_ERROR(
                    merge_ctx->m_import_ctx->m_em, "plugin_tiledmap_manip_import_merge: create tmp pixel buf fail, width=%d, height=%d!",
                    width, height);
                return -1;
            }
        }

        level_info = ui_cache_pixel_buf_level_info_at(merge_ctx->m_tmp_pixel_buf, 0);
        assert(level_info);

        if (ui_cache_pixel_buf_level_width(level_info) < width || ui_cache_pixel_buf_level_height(level_info) < height) {
            if (ui_cache_pixel_buf_pixel_buf_create(merge_ctx->m_tmp_pixel_buf, width, height, ui_cache_pf_r8g8b8a8, 1) != 0) {
                CPE_ERROR(
                    merge_ctx->m_import_ctx->m_em, "plugin_tiledmap_manip_import_merge: create tmp pixel buf fail, width=%d, height=%d!",
                    width, height);
                return -1;
            }
        }

        tmp_rect.level = 0;
        tmp_rect.boundary_lt = 0;
        tmp_rect.boundary_tp = 0;
        tmp_rect.boundary_rt = width;
        tmp_rect.boundary_bm = height;

        if (ui_cache_pixel_buf_rect_op(
                merge_ctx->m_tmp_pixel_buf, &tmp_rect, tile->m_pixel_buf, &tile->m_rect,
                (ui_cache_pixel_rect_flip_type_t)flip_type, (ui_cache_pixel_rect_angle_type_t)angle_type, merge_ctx->m_import_ctx->m_em)
            != 0)
        {
            CPE_ERROR(
                merge_ctx->m_import_ctx->m_em, "plugin_tiledmap_manip_import_merge: copy data to tmp pixel buf fail, width=%d, height=%d!",
                width, height);
            return -1;
        }

        if (ui_cache_pixel_buf_rect_md5(&md5_value, merge_ctx->m_tmp_pixel_buf, &tmp_rect, merge_ctx->m_import_ctx->m_em) != 0) {
            CPE_ERROR(merge_ctx->m_import_ctx->m_em, "plugin_tiledmap_manip_import_merge: calc img rect md5 from tmp buf fail!");
            return -1;
        }
    }
    else {
        if (ui_cache_pixel_buf_rect_md5(&md5_value, tile->m_pixel_buf, &tile->m_rect, merge_ctx->m_import_ctx->m_em) != 0) {
            CPE_ERROR(merge_ctx->m_import_ctx->m_em, "plugin_tiledmap_manip_import_merge: calc img rect md5 fail!");
            return -1;
        }
    }

    /* struct mem_buffer buff; */
    /* mem_buffer_init(&buff, NULL); */
    /* printf("xxxxx: tile=%p, flip=%d, angle=%d: md5=%s\n", tile, flip_type, angle_type, cpe_md5_dump(&md5_value, &buff)); */
    /* mem_buffer_clear(&buff); */

    merge_tile = plugin_tiledmap_manip_import_merge_tile_find(merge_ctx, &md5_value);
    if (merge_tile) {
        float adj_x = 0.0f;
        float adj_y = 0.0f;

        ui_cache_pixel_op_netative(&flip_type, &angle_type);
        assert(ui_cache_pixel_op_is_regular(flip_type, angle_type));

        switch(flip_type) {
        case tiledmap_tile_flip_type_none:
            if (angle_type == tiledmap_tile_angle_type_90) {
                adj_x = width;
            }
            break;
        case tiledmap_tile_flip_type_x:
            if (angle_type == tiledmap_tile_angle_type_90) {
                adj_x = width;
                adj_y = height;
            }
            else {
                adj_x = width;
            }
            break;
        case tiledmap_tile_flip_type_y:
            if (angle_type == tiledmap_tile_angle_type_90) {
            }
            else {
                adj_y = height;
            }
            break;
        case tiledmap_tile_flip_type_xy:
            if (angle_type == tiledmap_tile_angle_type_90) {
                adj_y = height;
            }
            else {
                adj_x = width;
                adj_y = height;
            }
            break;
        default:
            break;
        }
        
        while(!TAILQ_EMPTY(&tile->m_refs)) {
            plugin_tiledmap_manip_import_tile_ref_t tile_ref = TAILQ_FIRST(&tile->m_refs);
            plugin_tiledmap_manip_import_tile_ref_set_tile(tile_ref, merge_tile->m_tile);

            tile_ref->m_x += adj_x;
            tile_ref->m_y += adj_y;
            tile_ref->m_flip_type = flip_type;
            tile_ref->m_angle_type = angle_type;
        }
    }
    else {
        if (flip_type == tiledmap_tile_flip_type_none && angle_type == tiledmap_tile_angle_type_none) {
            merge_tile = plugin_tiledmap_manip_import_merge_tile_create(merge_ctx, &md5_value, tile);
            if (merge_tile == NULL) {
                CPE_ERROR(merge_ctx->m_import_ctx->m_em, "plugin_tiledmap_manip_import_merge: create merge tile fail!");
                return -1;
            }
        }
    }

    return 0;
}
