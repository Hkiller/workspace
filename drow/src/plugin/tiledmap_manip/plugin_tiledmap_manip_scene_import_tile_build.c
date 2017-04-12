#include <assert.h>
#include "cpe/utils/binpack.h"
#include "cpe/utils/file.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"

int plugin_tiledmap_manip_import_tile_build_module(plugin_tiledmap_manip_import_ctx_t ctx) {
    plugin_tiledmap_manip_import_tile_t tile;
    uint32_t output_width = 0;
    uint32_t output_height = 0;
    binpack_maxrects_ctx_t binpack = NULL;
    ui_cache_pixel_buf_t pixel_buf = NULL;
    struct binpack_rect * dst = NULL;
    uint32_t dst_size = 0;
    struct binpack_rect_size * rects = NULL;
    uint32_t i;
    uint32_t rects_size = 0;
    int rv = -1;
    /* ui_ed_obj_t module_obj; */
    /* UI_MODULE * module_data; */
    ui_ed_obj_t img_block_obj;
    UI_IMG_BLOCK * img_block;
    struct ui_cache_pixel_buf_rect to_rect;

    /*计算tile总数 */
    TAILQ_FOREACH(tile, &ctx->m_tiles, m_next_for_ctx) {
        ++rects_size;
    }

    /*构造输入数组 */
    rects = mem_alloc(ctx->m_alloc, sizeof(struct binpack_rect_size) * rects_size);
    if (rects == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: alloc rects fail, size=%d!", ctx->m_proj_path, rects_size);
        goto COMPLETE;
    }

    i = 0;
    TAILQ_FOREACH(tile, &ctx->m_tiles, m_next_for_ctx) {
        rects[i].width = tile->m_rect.boundary_rt - tile->m_rect.boundary_lt;
        rects[i].height = tile->m_rect.boundary_bm - tile->m_rect.boundary_tp;
        rects[i].ctx = tile;
        ++i;
    }

    /*分配输出数组 */
    dst = mem_alloc(ctx->m_alloc, sizeof(struct binpack_rect) * rects_size);
    if (dst == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: alloc dst fail, size=%d!", ctx->m_proj_path, rects_size);
        goto COMPLETE;
    }
                                         
    /*放置 */
    binpack = binpack_maxrects_ctx_create(ctx->m_alloc, ctx->m_em);
    if (binpack == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: create binpack fail!", ctx->m_proj_path);
        goto COMPLETE;
    }

    if (binpack_maxrects_ctx_init(binpack, ctx->m_max_width, ctx->m_max_height) != 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: build module: binpack init (%d,%d) fail!",
            ctx->m_proj_path, ctx->m_max_width, ctx->m_max_height);
        goto COMPLETE;
    }

    /*
    binpack_maxrects_best_short_side_fit
    binpack_maxrects_best_lone_side_fit
    binpack_maxrects_best_lone_area_fit
    binpack_maxrects_bottom_left_rule
    binpack_maxrects_contact_point_rule
    */
    if (binpack_maxrects_ctx_bulk_insert(
            binpack,
            dst, &dst_size,
            rects, &rects_size, binpack_maxrects_contact_point_rule, 0)
        != 0)
    {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: build module: binpack fail, limit=(%d,%d), dst_size=%d, rects_size=%d!",
            ctx->m_proj_path, ctx->m_max_width, ctx->m_max_height, dst_size, rects_size);
        goto COMPLETE;
    }

    /*构造图片 */
    pixel_buf = ui_cache_pixel_buf_create(ctx->m_cache_mgr);
    if (pixel_buf == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: create pixel buf fail!", ctx->m_proj_path);
        goto COMPLETE;
    }

    /*  计算最大输出图片包围框 */
    output_width = 0;
    output_height = 0;
    for(i = 0; i < dst_size; ++i) {
        binpack_rect_t rect = dst + i;
        if (rect->y + rect->height > output_height) output_height = rect->y + rect->height;
        if (rect->x + rect->width > output_width) output_width = rect->x + rect->width;
    }

    if (output_height == 0 || output_width == 0) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: output size width=%d height=%d error!", ctx->m_proj_path, output_height, output_width);
        goto COMPLETE;
    }

    for(i = 16; i < 0xFFFFFFFF; i *= 2) {
        if (i > output_width) {
            output_width = i;
            break;
        }
    }

    for(i = 16; i < 0xFFFFFFFF; i *= 2) {
        if (i > output_height) {
            output_height = i;
            break;
        }
    }

    if (ui_cache_pixel_buf_pixel_buf_create(pixel_buf, output_width, output_height, ui_cache_pf_r8g8b8a8, 1) != 0) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: alloc pixel buf data fail!", ctx->m_proj_path);
        goto COMPLETE;
    }

    to_rect.level = 0;
    to_rect.boundary_lt = 0;
    to_rect.boundary_tp = 0;
    to_rect.boundary_rt = output_width;
    to_rect.boundary_bm = output_height;

    if (ui_cache_pixel_buf_rect_clear(pixel_buf, &to_rect, ctx->m_em) != 0) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: clear rect fail!", ctx->m_proj_path);
        goto COMPLETE;
    }

    for(i = 0; i < dst_size; ++i) {
        binpack_rect_t rect = dst + i;
        tile = rect->ctx;

        to_rect.level = 0;
        to_rect.boundary_lt = rect->x;
        to_rect.boundary_tp = rect->y;
        to_rect.boundary_rt = to_rect.boundary_lt + rect->width;
        to_rect.boundary_bm = to_rect.boundary_tp + rect->height;

        if (ui_cache_pixel_buf_rect_copy(pixel_buf, &to_rect, tile->m_pixel_buf, &tile->m_rect, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "scene import from %s: build module: copy rect fail!", ctx->m_proj_path);
            goto COMPLETE;
        }
    }

    /*构造Module */
    ctx->m_tile_model_src = ui_ed_src_check_create(ctx->m_ed_mgr, ctx->m_tile_path, ui_data_src_type_module);
    if (ctx->m_tile_model_src == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build module: check create tile src at %s fail!", ctx->m_proj_path, ctx->m_tile_path);
        goto COMPLETE;
    }

    /* module_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(ctx->m_tile_model_src)); */
    /* assert(module_obj); */
    /* ui_ed_obj_remove_childs(module_obj); */

    /*TODO: Loki*/
    assert(0);
    /* module_data = ui_ed_obj_data(module_obj); */
    /* snprintf(module_data->use_img, sizeof(module_data->use_img), "%s.png", ctx->m_tile_path); */

    /* for(i = 0; i < dst_size; ++i) { */
    /*     binpack_rect_t rect = dst + i; */
    /*     tile = rect->ctx; */

    /*     img_block_obj = ui_ed_obj_new(module_obj, ui_ed_obj_type_img_block); */
    /*     if (img_block_obj == NULL) { */
    /*         CPE_ERROR(ctx->m_em, "scene import from %s: build module: create img block fail!", ctx->m_proj_path); */
    /*         goto COMPLETE; */
    /*     } */

    /*     tile->m_img_block_id = i + 1; */
    /*     if (ui_ed_obj_set_id(img_block_obj, tile->m_img_block_id) != 0) { */
    /*         CPE_ERROR(ctx->m_em, "scene import from %s: build module: set img block id %d fail!", ctx->m_proj_path, tile->m_img_block_id); */
    /*         goto COMPLETE; */
    /*     } */
        
    /*     img_block = ui_ed_obj_data(img_block_obj); */
    /*     assert(img_block); */

    /*     img_block->src_x = rect->x; */
    /*     img_block->src_y = rect->y; */
    /*     img_block->src_w = rect->width; */
    /*     img_block->src_h = rect->height; */
    /*     img_block->flag = 1; */
    /* } */

    rv = 0;

COMPLETE:
    if (binpack) {
        binpack_maxrects_ctx_free(binpack);
        binpack = NULL;
    }

    if (dst) {
        mem_free(ctx->m_alloc, dst);
        dst = NULL;
    }
    
    if (rects) {
        mem_free(ctx->m_alloc, rects);
        rects = NULL;
    }

    if (pixel_buf) {
        if (rv == 0) {
            assert(ctx->m_tile_pixel_buf == NULL);
            ctx->m_tile_pixel_buf = pixel_buf;
        }
        else {
            ui_cache_pixel_buf_free(pixel_buf);
        }

        pixel_buf = NULL;
    }
    
    return rv;
}
