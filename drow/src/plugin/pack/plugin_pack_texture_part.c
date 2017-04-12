#include <assert.h>
#include "cpe/utils/binpack.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "plugin_pack_texture_part_i.h"
#include "plugin_pack_block_i.h"
#include "plugin_pack_block_ref_i.h"

plugin_pack_texture_part_t
plugin_pack_texture_part_create(plugin_pack_texture_t texture) {
    plugin_pack_packer_t packer = texture->m_packer;
    plugin_pack_texture_part_t part;

    part = mem_alloc(packer->m_module->m_alloc, sizeof(struct plugin_pack_texture_part));
    if (part == NULL) {
        CPE_ERROR(packer->m_module->m_em, "plugin_pack_packer: part: create: alloc fail!");
        return NULL;
    }

    part->m_texture = texture;
    part->m_binpack = NULL;
    TAILQ_INIT(&part->m_blocks);

    texture->m_part_count++;
    TAILQ_INSERT_TAIL(&texture->m_parts, part, m_next);
    
    return part;
}

void plugin_pack_texture_part_free(plugin_pack_texture_part_t texture_part) {
    plugin_pack_texture_t texture = texture_part->m_texture;
    plugin_pack_packer_t packer = texture->m_packer;

    assert(TAILQ_EMPTY(&texture_part->m_blocks));

    if (texture_part->m_binpack) {
        binpack_maxrects_ctx_free(texture_part->m_binpack);
        texture_part->m_binpack = NULL;
    }

    assert(texture->m_part_count > 0);
    texture->m_part_count--;

    TAILQ_REMOVE(&texture->m_parts, texture_part, m_next);

    mem_free(packer->m_module->m_alloc, texture_part);
}

int plugin_pack_texture_part_commit(plugin_pack_texture_part_t part, const char * path) {
    plugin_pack_packer_t packer = part->m_texture->m_packer;
    plugin_pack_module_t module = packer->m_module;
    plugin_pack_block_t block;
    ui_cache_res_t texture = NULL;
    ui_cache_pixel_buf_t pixel_buf = NULL;
    struct ui_cache_pixel_buf_rect full_rect;

    texture = ui_cache_res_create(module->m_cache_mgr, ui_cache_res_type_texture);
    if (texture == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: create cache texture fail!");
        return -1;
    }
    
    if (ui_cache_res_set_path(texture, path) != 0) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: set texture path fail!");
        ui_cache_res_free(texture);
        return -1;
    }

    if (ui_cache_group_add_res(packer->m_generated_textures, texture) != 0) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: texture %s add to generated fail!", path);
        ui_cache_res_free(texture);
        return -1;
    }

    pixel_buf = ui_cache_pixel_buf_create(module->m_cache_mgr);
    if (pixel_buf == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: create pixel buf fail!");
        ui_cache_res_free(texture);
        return -1;
    }

    if (ui_cache_pixel_buf_pixel_buf_create(
            pixel_buf,
            binpack_maxrects_ctx_width(part->m_binpack),
            binpack_maxrects_ctx_height(part->m_binpack),
            ui_cache_pf_r8g8b8a8, 1) != 0)
    {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: alloc pixel buf data fail!");
        ui_cache_pixel_buf_free(pixel_buf);
        ui_cache_res_free(texture);
        return -1;
    }

    full_rect.level = 0;
    full_rect.boundary_lt = 0;
    full_rect.boundary_tp = 0;
    full_rect.boundary_rt = binpack_maxrects_ctx_width(part->m_binpack);
    full_rect.boundary_bm = binpack_maxrects_ctx_height(part->m_binpack);

    if (ui_cache_pixel_buf_rect_clear(pixel_buf, &full_rect, module->m_em) != 0) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: clear pixel buf fail!");
        ui_cache_pixel_buf_free(pixel_buf);
        ui_cache_res_free(texture);
        return -1;
    }
    
    if (ui_cache_texture_attach_data_buf(texture, pixel_buf) != 0) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_part_commit: commit: attach pixel buf fail!");
        ui_cache_pixel_buf_free(pixel_buf);
        ui_cache_res_free(texture);
        return -1;
    }
    
    TAILQ_FOREACH(block, &part->m_blocks, m_next_for_part) {
        plugin_pack_block_ref_t block_ref;

        if (ui_cache_pixel_buf_rect_copy(
                pixel_buf, &block->m_to_rect,
                ui_cache_texture_data_buf(block->m_src_texture), &block->m_src_rect, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "plugin_pack_texture_part_commit: copy rect fail, src=(%d,%d)~(%d,%d), target=(%d,%d)~(%d,%d)!",
                block->m_src_rect.boundary_lt, block->m_src_rect.boundary_tp, block->m_src_rect.boundary_rt, block->m_src_rect.boundary_bm,
                block->m_to_rect.boundary_lt, block->m_to_rect.boundary_tp, block->m_to_rect.boundary_rt, block->m_to_rect.boundary_bm);
            ui_cache_res_free(texture);
            return -1;
        }
        
        TAILQ_FOREACH(block_ref, &block->m_refs, m_next) {
            switch(ui_ed_obj_type_id(block_ref->m_ed_obj)) {
            case ui_ed_obj_type_img_block:
                ui_ed_src_touch(ui_ed_obj_src(block_ref->m_ed_obj));
                plugin_pack_packer_update_img_block(block_ref, path);
                break;
            case ui_ed_obj_type_particle_emitter:
                ui_ed_src_touch(ui_ed_obj_src(block_ref->m_ed_obj));
                plugin_pack_packer_update_particle_emitter(block_ref, path);
                break;
            default:
                break;
            }
        }
    }

    return 0;
}

static int plugin_pack_texture_part_create_pack(plugin_pack_texture_part_t part, uint32_t width, uint32_t height) {
    plugin_pack_packer_t packer = part->m_texture->m_packer;
    plugin_pack_module_t module = packer->m_module;
    binpack_maxrects_ctx_t old_binpack = part->m_binpack;
    plugin_pack_block_t block;
    
    part->m_binpack = binpack_maxrects_ctx_create(module->m_alloc, module->m_em);
    if (part->m_binpack == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_texture_bin_pack: create binpack fail!");
        part->m_binpack = old_binpack;
        return -1;
    }
    binpack_maxrects_ctx_set_span(part->m_binpack, packer->m_texture_span);

    if (binpack_maxrects_ctx_init(part->m_binpack, width, height) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_pack_texture_bin_pack: binpack init (%d,%d) fail!",
            width, height);
        binpack_maxrects_ctx_free(part->m_binpack);
        part->m_binpack = old_binpack;
        return -1;
    }

    /* if (old_binpack) { */
    /*     printf( */
    /*         " xxxxx: limit=(%d,%d), resize-size (%d,%d) ==> (%d,%d)\n", */
    /*         packer->m_limit_width, packer->m_limit_height, */
    /*         binpack_maxrects_ctx_width(old_binpack), binpack_maxrects_ctx_height(old_binpack), */
    /*         width, height); */
    /* } */
    /* else { */
    /*     printf( */
    /*         " xxxxx: limit=(%d,%d), init-size=(%d,%d)\n", */
    /*         packer->m_limit_width, packer->m_limit_height, */
    /*         width, height); */
    /* } */
    
    TAILQ_FOREACH(block, &part->m_blocks, m_next_for_part) {
        uint32_t place_width = block->m_to_rect.boundary_rt - block->m_to_rect.boundary_lt;
        uint32_t place_height = block->m_to_rect.boundary_bm - block->m_to_rect.boundary_tp;
        binpack_rect_t placed_pack = NULL;

        /* printf("             place (%d,%d)\n", place_width, place_height); */
        placed_pack = binpack_maxrects_ctx_insert(
            part->m_binpack, place_width, place_height, binpack_maxrects_best_short_side_fit, 0);
        assert(placed_pack);

        block->m_to_rect.level = 0;
        block->m_to_rect.boundary_lt = placed_pack->x;
        block->m_to_rect.boundary_tp = placed_pack->y;
        block->m_to_rect.boundary_rt = block->m_to_rect.boundary_lt + placed_pack->width;
        block->m_to_rect.boundary_bm = block->m_to_rect.boundary_tp + placed_pack->height;
    }
    
    return 0;
}

static int plugin_pack_texture_part_try_resize(
    plugin_pack_packer_t packer,
    uint32_t * width, uint32_t * height,
    uint32_t place_width, uint32_t place_height)
{
    uint8_t resized = 0;
    
    while (place_width > *width) {
        *width *= 2;
        resized = 1;
    }

    while(place_height > *height) {
        *height *= 2;
        resized = 1;
    }

    assert(packer->m_limit_width == 0 || *width <= packer->m_limit_width);
    assert(packer->m_limit_height == 0 || *height <= packer->m_limit_height);

    if (resized) return 0;
    
    if (*width <= *height) {
        if (packer->m_limit_width == 0 || *width * 2 <= packer->m_limit_width) {
            *width *= 2;
            resized = 1;
        }
    }
    else {
        if (packer->m_limit_height == 0 || *height * 2 <= packer->m_limit_height) {
            *height *= 2;
            resized = 1;
        }
    }

    return resized ? 0 : -1;
}
    
plugin_pack_texture_part_t
plugin_pack_texture_alloc(plugin_pack_texture_t texture, ui_cache_pixel_buf_rect_t rect, binpack_rect_t o_placed_pack) {
    plugin_pack_packer_t packer = texture->m_packer;
    plugin_pack_texture_part_t part;
    uint32_t place_width, place_height;
    binpack_rect_t placed_pack = NULL;
    uint32_t next_width;
    uint32_t next_height;

    place_width = rect->boundary_rt - rect->boundary_lt;
    place_height = rect->boundary_bm - rect->boundary_tp;

    /*检查大小整体不能超过限制 */
    if (packer->m_limit_width && place_width > packer->m_limit_width) return NULL;
    if (packer->m_limit_height && place_height > packer->m_limit_height) return NULL;

    /*遍历已经有的part， 尝试放置进去 */
    TAILQ_FOREACH(part, &texture->m_parts, m_next) {
    TRY_ALLOC_AGAIN:
        /*如果存在，则放置进去 */
        if (part->m_binpack) {
            placed_pack = binpack_maxrects_ctx_insert(part->m_binpack, place_width, place_height, binpack_maxrects_best_short_side_fit, 0);
            if (placed_pack) {
                *o_placed_pack = *placed_pack;
                return part;
            }
        }
        /*放置失败，尝试创建或者增加大小 */
        assert(part->m_binpack);
        next_width = binpack_maxrects_ctx_width(part->m_binpack);
        next_height = binpack_maxrects_ctx_height(part->m_binpack);

        /*寻找下一个大小失败，则跳过这个part */
        if (plugin_pack_texture_part_try_resize(packer, &next_width, &next_height, place_width, place_height) != 0) continue;

        assert(next_width > binpack_maxrects_ctx_width(part->m_binpack)
               || next_height > binpack_maxrects_ctx_height(part->m_binpack));
        
        if (plugin_pack_texture_part_create_pack(part, next_width, next_height) != 0) continue;

        assert(part->m_binpack);
        placed_pack = binpack_maxrects_ctx_insert(part->m_binpack, place_width, place_height, binpack_maxrects_best_short_side_fit, 0);
        if (placed_pack == NULL) goto TRY_ALLOC_AGAIN;
        
        *o_placed_pack = *placed_pack;
        return part;
    }

    /*设置初始大小 */
    next_width = 512;
    next_height = 512;

    if (place_width > next_width || place_height > next_height) {
        /*需要放置的块已经越界，尝试下一个大小 */
        if (plugin_pack_texture_part_try_resize(packer, &next_width, &next_height, place_width, place_height) != 0) {
            /*已经保护过整体越界了，绝对不可能到此 */
            assert(0);
        }
    }
    
    part = plugin_pack_texture_part_create(texture);
    if (part == NULL) return NULL;

    if (plugin_pack_texture_part_create_pack(part, next_width, next_height) != 0) {
        plugin_pack_texture_part_free(part);
        return NULL;
    }

    placed_pack = binpack_maxrects_ctx_insert(part->m_binpack, place_width, place_height, binpack_maxrects_best_short_side_fit, 0);
    if (placed_pack == NULL) goto TRY_ALLOC_AGAIN;
    
    *o_placed_pack = *placed_pack;
    return part;
}

