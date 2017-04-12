#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/binpack.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "render/model_manip/model_id_allocrator.h"
#include "render/model_manip/model_manip_basic.h"

struct ui_model_import_block {
    char m_name[64];
    ui_cache_res_t m_input_texture;
    uint32_t m_id;
    struct ui_cache_pixel_buf_rect m_to_rect;
};

static int ui_model_module_import_build_blocks(
    struct ui_model_import_block * blocks, uint32_t * block_count,
    ui_cache_manager_t cache_mgr, ui_model_id_allocrator_t id_alloc,
    const char * base_dir, const char * * srcs, uint32_t src_count,
    error_monitor_t em);

static int ui_model_module_import_pack(
    gd_app_context_t app, ui_cache_manager_t cache_mgr, ui_cache_res_t texture,
    struct ui_model_import_block * blocks, uint32_t block_count,
    error_monitor_t em);

static int ui_model_module_import_build_buf(
    gd_app_context_t app, ui_cache_manager_t cache_mgr, ui_cache_res_t texture,
    uint32_t width, uint32_t height, binpack_rect_t dst, uint32_t dst_size, error_monitor_t em);

int ui_model_import_module(
    ui_ed_mgr_t ed_mgr, ui_cache_manager_t cache_mgr,
    const char * to_module, const char * base_dir, const char * * srcs, uint32_t src_count,
    error_monitor_t em)
{
    gd_app_context_t app = ui_ed_mgr_app(ed_mgr);
    ui_data_mgr_t data_mgr = ui_ed_mgr_data_mgr(ed_mgr);
    ui_data_module_t module;
    ui_ed_src_t module_src;
    uint32_t i;
    ui_model_id_allocrator_t id_alloc = NULL;
    struct ui_model_import_block * blocks = NULL;
    uint32_t block_count;
    char pic_path[512];
    ui_cache_res_t target_texture = NULL;
    struct ui_ed_obj_it child_obj_it;
    ui_ed_obj_t child_obj, next_child_obj;
    int rv = -1;
    
    snprintf(pic_path, sizeof(pic_path), "%s.png", to_module);
    
    blocks = mem_alloc(gd_app_alloc(app), sizeof(struct ui_model_import_block) * src_count);
    if (blocks == NULL) {
        CPE_ERROR(em, "ui_model_import_module: allock block buf fail!");
        goto IMPORT_ERROR;
    }
    
    module_src = ui_ed_src_check_create(ed_mgr, to_module, ui_data_src_type_module);
    if (module_src == NULL) {
        CPE_ERROR(em, "check create module at %s fail!", to_module);
        goto IMPORT_ERROR;
    }

    module = ui_data_src_product(ui_ed_src_data(module_src));
    assert(module);

    /*分配ID */
    id_alloc = ui_model_id_allocrator_create(NULL);
    if (id_alloc == NULL) {
        CPE_ERROR(em, "create model id alloc fail!");
        goto IMPORT_ERROR;
    }

    /*初始化block列表 */
    if (ui_model_module_import_build_blocks(blocks, &block_count, cache_mgr, id_alloc, base_dir, srcs, src_count, em) != 0) goto IMPORT_ERROR;

    /*   其次根据已经分配好的ID进行放置 */
    for(i = 0; i < block_count; ++i) {
        struct ui_model_import_block * block = blocks + i;
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK * img_block_data;

        if (blocks->m_id != (uint32_t)-1) continue;

        img_block = ui_data_img_block_find_by_name(module, block->m_name);
        if (img_block == NULL) continue;
        img_block_data = ui_data_img_block_data(img_block);

        if (ui_model_id_allocrator_remove(id_alloc, img_block_data->id) != 0) {
            CPE_ERROR(em, "id %u [modlue old id] (name=%s) duplicate!", img_block_data->id, block->m_name);
            continue;
        }

        block->m_id = img_block_data->id;
    }

    /*    最后给所以没有生成的ID进行分配 */
    for(i = 0; i < block_count; ++i) {
        struct ui_model_import_block * block = blocks + i;
        uint32_t img_id;
        
        if (block->m_id != (uint32_t)-1) continue;
        
        if (ui_model_id_allocrator_alloc(id_alloc, &img_id) != 0) {
            CPE_ERROR(em, "allock (name=%s) fail!", block->m_name);
            continue;
        }

        block->m_id = img_id;
    }

    /*创建图片 */
    target_texture = ui_cache_res_find_by_path(cache_mgr, pic_path);
    if (target_texture == NULL) {
        target_texture = ui_cache_res_create(cache_mgr, ui_cache_res_type_texture);
        if (target_texture == NULL) {
            CPE_ERROR(em, "import_module: target texture %s create fail", pic_path);
            goto IMPORT_ERROR;
        }

        if (ui_cache_res_set_path(target_texture, pic_path) != 0) {
            CPE_ERROR(em, "import_module: target texture %s set path fail", pic_path);
            goto IMPORT_ERROR;
        }
    }

    if (ui_model_module_import_pack(app, cache_mgr, target_texture, blocks, block_count, em) != 0) {
        goto IMPORT_ERROR;
    }

    /*构造最终的block */
    ui_ed_obj_childs(&child_obj_it, ui_ed_src_root_obj(module_src));
    for(child_obj = ui_ed_obj_it_next(&child_obj_it); child_obj; child_obj = next_child_obj) {
        next_child_obj = ui_ed_obj_it_next(&child_obj_it);
        
        if (ui_ed_obj_type_id(child_obj) == ui_ed_obj_type_img_block) {
            UI_IMG_BLOCK * img_block = ui_ed_obj_data(child_obj);
            ui_ed_src_msg_remove(module_src, img_block->name_id);
            ui_ed_obj_remove(child_obj);
        }
    }
    
    for(i = 0; i < block_count; ++i) {
        struct ui_model_import_block * block = blocks + i;
        ui_ed_obj_t img_block_obj;
        UI_IMG_BLOCK * img_block;
        
        img_block_obj = ui_ed_obj_new(ui_ed_src_root_obj(module_src), ui_ed_obj_type_img_block);
        if (img_block_obj == NULL) {
            CPE_ERROR(em, "module_import: create img_block fail!");
            goto IMPORT_ERROR;
        }

        if (ui_ed_obj_set_id(img_block_obj, block->m_id) != 0) {
            CPE_ERROR(em, "module_import:: set id %d fail!", block->m_id);
            goto IMPORT_ERROR;
        }

        img_block = ui_ed_obj_data(img_block_obj);
        assert(img_block);

        /*填写img_block*/
        img_block->name_id = ui_ed_src_msg_alloc(module_src, block->m_name);
        img_block->use_img = ui_ed_src_msg_alloc(module_src, pic_path);
        img_block->src_x = block->m_to_rect.boundary_lt;
        img_block->src_y = block->m_to_rect.boundary_tp;
        img_block->src_w = block->m_to_rect.boundary_rt - block->m_to_rect.boundary_lt;
        img_block->src_h = block->m_to_rect.boundary_bm - block->m_to_rect.boundary_tp;
        img_block->flag = 1;
    }

    snprintf(pic_path, sizeof(pic_path), "%s/%s", ui_data_src_data(ui_data_mgr_src_root(data_mgr)), ui_cache_res_path(target_texture));
    if (ui_cache_pixel_buf_save_to_file(ui_cache_texture_data_buf(target_texture), pic_path, em, NULL) != 0) {
        CPE_ERROR(em, "module_import:: save texture to %s fail!", pic_path);
        goto IMPORT_ERROR;
    }

    rv = 0;

IMPORT_ERROR:
    if (rv != 0) {
        if (module_src) {
            ui_ed_src_delete(module_src);
            module_src = NULL;
        }
    }

    if (id_alloc) {
        ui_model_id_allocrator_free(id_alloc);
        id_alloc = NULL;
    }

    if (blocks) {
        mem_free(gd_app_alloc(app), blocks);
        blocks = NULL;
    }

    return rv;
}

static int ui_model_module_import_build_blocks(
    struct ui_model_import_block * r_blocks, uint32_t * r_block_count,
    ui_cache_manager_t cache_mgr, ui_model_id_allocrator_t id_alloc,
    const char * base_dir, const char * * srcs, uint32_t src_count,
    error_monitor_t em)
{
    uint32_t block_count = 0;
    uint32_t i;
    int rv = 0;
    
    for(i = 0; i < src_count; ++i) {
        struct ui_model_import_block * block = r_blocks + block_count;
        const char * name_begin;
        const char * name_end;
        uint32_t img_id;
        char * end_p;
        ui_cache_pixel_buf_t source_texture_buf;
        
        if (base_dir && cpe_str_start_with(srcs[i], base_dir)) {
            name_begin = srcs[i] + strlen(base_dir);
        }
        else {
            name_begin = file_name_no_dir(srcs[i]);
        }

        name_end = strrchr(name_begin, '.');
        if (name_end && name_end > name_begin) {
            cpe_str_dup_range(block->m_name, sizeof(block->m_name), name_begin, name_end);
        }
        else {
            cpe_str_dup(block->m_name, sizeof(block->m_name), name_begin);
        }
        
        block->m_input_texture = ui_cache_res_find_by_path(cache_mgr, srcs[i]);
        if (block->m_input_texture == NULL) {
            block->m_input_texture = ui_cache_res_create(cache_mgr, ui_cache_res_type_texture);
            if (block->m_input_texture == NULL) {
                CPE_ERROR(em, "import_module: src %s create fail", srcs[i]);
                rv = -1;
                continue;
            }

            if (ui_cache_res_set_path(block->m_input_texture, srcs[i]) != 0) {
                CPE_ERROR(em, "import_module: src %s set path fail", srcs[i]);
                rv = -1;
                continue;
            }
        }

        source_texture_buf = ui_cache_texture_data_buf(block->m_input_texture);
        if (source_texture_buf == NULL) {
            ui_cache_texture_set_keep_data_buf(block->m_input_texture, 1);
            if (ui_cache_res_load_sync(block->m_input_texture, NULL) != 0) {
                CPE_ERROR(em, "import_module: load %s fail!", ui_cache_res_path(block->m_input_texture));
                return -1;
            }

            source_texture_buf = ui_cache_texture_data_buf(block->m_input_texture);
            if (source_texture_buf == NULL) {
                CPE_ERROR(em, "import_module: load %s success, no data buf!", ui_cache_res_path(block->m_input_texture));
                return -1;
            }
        
            assert(source_texture_buf);
        }

        /*读取id */
        img_id = strtol(block->m_name, &end_p, 10);
        if (end_p && (*end_p == '_' || (*end_p == '.' && strchr(end_p + 1, '.') == NULL))) {
            if (ui_model_id_allocrator_remove(id_alloc, img_id) != 0) {
                CPE_ERROR(em, "import_module: id %u [file name id] (name=%s) duplicate!", img_id, block->m_name);
                rv = -1;
                block->m_id = -1;
            }
            else {
                block->m_id = img_id;
            }
        }
        else {
            block->m_id = (uint32_t)-1;
        }

        block_count++;
    }

    *r_block_count = block_count;
    
    return rv;
}

static int ui_model_module_import_pack(
    gd_app_context_t app, ui_cache_manager_t cache_mgr, ui_cache_res_t texture,
    struct ui_model_import_block * blocks, uint32_t block_count,
    error_monitor_t em)
{
    uint32_t block_max_width;
    uint32_t block_max_height;
    uint64_t block_total_area;
    uint32_t output_width;
    uint32_t output_height;
    binpack_maxrects_ctx_t binpack = NULL;
    struct binpack_rect * dst = NULL;
    uint32_t dst_size = 0;
    struct binpack_rect_size * rects = NULL;
    uint32_t i;
    int rv = -1;

    /*  计算最大输出图片包围框 */
    block_max_width = 0;
    block_max_height = 0;
    block_total_area = 0;

    for(i = 0; i < block_count; ++i) {
        struct ui_model_import_block * block = blocks + i;
        uint32_t width = ui_cache_texture_width(block->m_input_texture);
        uint32_t height = ui_cache_texture_height(block->m_input_texture);
        uint64_t area = (uint64_t)width * (uint64_t)height;

        if (width > block_max_width) block_max_width = width;
        if (height > block_max_height) block_max_height = height;
        block_total_area += area;
    }

    if (block_total_area == 0) {
        CPE_ERROR(em, "module_import: block no area!");
        return -1;
    }
    
    output_width = cpe_math_32_round_to_pow2((uint32_t)sqrt(block_total_area));
    if (output_width < block_max_width) output_width = cpe_math_32_round_to_pow2(block_max_width);
    output_height = cpe_math_32_round_to_pow2((uint32_t)(block_total_area / output_width));
    if (output_height < block_max_height) output_height = cpe_math_32_round_to_pow2(block_max_height);

    /* printf("\n\n\n begin pack %s: block-count=%d, area=%d, size=%d-%d, max_width=%d, max_height=%d\n", */
    /*        ui_cache_res_path(texture), block_count, */
    /*        (int)block_total_area, output_width, output_height, block_max_width, block_max_height); */

    /*构造输入数组  */
    rects = mem_alloc(gd_app_alloc(app), sizeof(struct binpack_rect_size) * block_count);
    if (rects == NULL) {
        CPE_ERROR(em, "module_import: alloc rects fail, size=%d!", block_count);
        goto COMPLETE;
    }

    /*分配输出数组 */
    dst = mem_alloc(gd_app_alloc(app), sizeof(struct binpack_rect) * block_count);
    if (dst == NULL) {
        CPE_ERROR(em, "module_import: alloc dts rects fail, size=%d!", block_count);
        goto COMPLETE;
    }
                                         
TRY_AGAIN2:
    /* 放置 */
    binpack = binpack_maxrects_ctx_create(gd_app_alloc(app), em);
    if (binpack == NULL) {
        CPE_ERROR(em, "module_import: create binpack fail!");
        goto COMPLETE;
    }
    binpack_maxrects_ctx_set_span(binpack, 1);

    if (binpack_maxrects_ctx_init(binpack, output_width, output_height) != 0) {
        CPE_ERROR(
            em, "module_import: binpack init (%d,%d) fail!",
            output_width, output_height);
        goto COMPLETE;
    }

    /*构造输入数组 */
    for(i = 0; i < block_count; ++i) {
        struct ui_model_import_block * block = blocks + i;
        rects[i].width = ui_cache_texture_width(block->m_input_texture);
        rects[i].height = ui_cache_texture_height(block->m_input_texture);
        rects[i].ctx = block;
    }
    
    /*根据大小放置 */
    if (binpack_maxrects_ctx_bulk_insert(
            binpack,
            dst, &dst_size,
            rects, &i, binpack_maxrects_best_short_side_fit, 0)
        != 0)
    {
        uint64_t cur_area = output_height * output_width;
        uint64_t left_area = 0;
        uint64_t next_area;
        uint32_t j;
        
        for(j = 0; j < i; ++j) {
            left_area += rects[j].width * rects[j].height;
        }

        next_area = cur_area * (1.0f + ((float)left_area) / ((float)cur_area));
        if (next_area > block_total_area * 3) {
            CPE_ERROR(
                em,
                "module_import: binpack fail, limit=(%d,%d), dst_size=%d, left rects_size=%d,"
                " cur_area=%d, left_area=%d, block_total_area=%d, next_area=%d, limit_area=%d!",
                output_width, output_height, dst_size, i,
                (int)cur_area, (int)left_area, (int)block_total_area, (int)next_area, (int)(block_total_area * 3));
            goto COMPLETE;
        }

        /* CPE_INFO( */
        /*     em, "begin pack %s: try again: next-area=%d, left-area=%d, cur-area=%d, block-count=%d, area=%d, size=%d-%d, max_width=%d, max_height=%d", */
        /*     ui_cache_res_path(texture), (int)next_area, (int)left_area, (int)cur_area, */
        /*     block_count, (int)block_total_area, output_width, output_height, block_max_width, block_max_height); */
        
        output_height = cpe_math_32_round_to_pow2((uint32_t)(next_area / output_width));
        if (output_height < block_max_height) output_height = cpe_math_32_round_to_pow2(block_max_height);
        binpack_maxrects_ctx_free(binpack);
        binpack = NULL;

        goto TRY_AGAIN2;
    }

    assert(dst_size == block_count);
    rv = ui_model_module_import_build_buf(app, cache_mgr, texture, output_width, output_height, dst, dst_size, em);

COMPLETE:
    if (binpack) binpack_maxrects_ctx_free(binpack);
    if (dst) mem_free(gd_app_alloc(app), dst);
    if (rects) mem_free(gd_app_alloc(app), rects);
    
    return rv;
}

static int ui_model_module_import_build_buf(
    gd_app_context_t app, ui_cache_manager_t cache_mgr, ui_cache_res_t texture,
    uint32_t width, uint32_t height, binpack_rect_t dst, uint32_t dst_size, error_monitor_t em)
{
    ui_cache_pixel_buf_t pixel_buf = NULL;
    struct ui_cache_pixel_buf_rect full_rect;
    uint32_t i;
    int rv = 0;
    
    pixel_buf = ui_cache_pixel_buf_create(cache_mgr);
    if (pixel_buf == NULL) {
        CPE_ERROR(em, "module_import: create pixel buf fail!");
        return -1;
    }

    if (ui_cache_pixel_buf_pixel_buf_create(pixel_buf, width, height, ui_cache_pf_r8g8b8a8, 1) != 0) {
        CPE_ERROR(em, "module_import: alloc pixel buf data fail!");
        return -1;
    }

    full_rect.level = 0;
    full_rect.boundary_lt = 0;
    full_rect.boundary_tp = 0;
    full_rect.boundary_rt = width;
    full_rect.boundary_bm = height;

    if (ui_cache_pixel_buf_rect_clear(pixel_buf, &full_rect, em) != 0) {
        CPE_ERROR(em, "module_import: clear rect fail!");
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }

    for(i = 0; i < dst_size; ++i) {
        binpack_rect_t pack_rect = dst + i;
        struct ui_model_import_block * block = pack_rect->ctx;
        struct ui_cache_pixel_buf_rect src_rect;

        src_rect.level = 0;
        src_rect.boundary_lt = 0;
        src_rect.boundary_tp = 0;
        src_rect.boundary_rt = ui_cache_texture_width(block->m_input_texture);
        src_rect.boundary_bm = ui_cache_texture_height(block->m_input_texture);

        block->m_to_rect.level = 0;
        block->m_to_rect.boundary_lt = pack_rect->x;
        block->m_to_rect.boundary_tp = pack_rect->y;
        block->m_to_rect.boundary_rt = block->m_to_rect.boundary_lt + pack_rect->width;
        block->m_to_rect.boundary_bm = block->m_to_rect.boundary_tp + pack_rect->height;

        if (ui_cache_pixel_buf_rect_copy(
                pixel_buf, &block->m_to_rect,
                ui_cache_texture_data_buf(block->m_input_texture), &src_rect, em)
            != 0)
        {
            CPE_ERROR(
                em, "module_import: %s: copy rect fail, src=(%d,%d)~(%d,%d), target=(%d,%d)~(%d,%d)!",
                block->m_name, src_rect.boundary_lt, src_rect.boundary_tp, src_rect.boundary_rt, src_rect.boundary_bm,
                block->m_to_rect.boundary_lt, block->m_to_rect.boundary_tp, block->m_to_rect.boundary_rt, block->m_to_rect.boundary_bm);
            rv = -1;
        }
    }

    if (rv == 0) {
        if (ui_cache_texture_attach_data_buf(texture, pixel_buf) != 0) {
            CPE_ERROR(em, "module_import: attach pixel buf fail!");
            ui_cache_pixel_buf_free(pixel_buf);
            return -1;
        }
    }

    return rv;
}
