#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_math.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/bitarry.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "plugin_mask_block_builder_i.h"
#include "plugin/mask/plugin_mask_data.h"

plugin_mask_block_builder_t
plugin_mask_block_builder_create(
    plugin_mask_manip_t manip, ui_cache_pixel_field_t source, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    plugin_mask_block_builder_t builder;
    uint32_t buf_size = width * height * sizeof(uint32_t);

    builder = mem_alloc(manip->m_alloc, sizeof(struct plugin_mask_block_builder) + buf_size);
    if (builder == NULL) {
        CPE_ERROR(manip->m_em, "plugin_mask_block_builder_create: alloc fail!");
        return NULL;
    }

    builder->m_manip = manip;
    builder->m_source = source;
    builder->m_x = x;
    builder->m_y = y;
    builder->m_width = width;
    builder->m_height = height;
    builder->m_lt_x = 0;
    builder->m_lt_y = 0;
    builder->m_rb_x = 0;
    builder->m_rb_y = 0;
    builder->m_have_data = 0;
    builder->m_buf = (void*)(builder + 1);
    bzero(builder->m_buf, buf_size);

    return builder;
}

void plugin_mask_block_builder_free(plugin_mask_block_builder_t builder) {
    mem_free(builder->m_manip->m_alloc, builder);
}

int plugin_mask_block_builder_create_block(
    plugin_mask_block_builder_t builder, ui_data_src_t mask_src, plugin_mask_data_t mask_data, const char * name) {
    plugin_mask_data_block_t block;
    unsigned char * buf;
    uint32_t i, j;
    uint32_t line_size;

    block = plugin_mask_data_block_create(
        mask_data,
        ui_data_src_msg_alloc(mask_src, name),
        builder->m_x, builder->m_y,
        builder->m_width, builder->m_height,
        builder->m_lt_x, builder->m_lt_y,
        builder->m_rb_x - builder->m_lt_x, builder->m_rb_y - builder->m_lt_y);
    if (block == NULL) {
        CPE_ERROR(builder->m_manip->m_em, "plugin_mask_block_builder_create_block: create block fail!");
        return -1;
    }

    buf = plugin_mask_data_block_check_create_buf(block);
    if (buf == NULL) {
        CPE_ERROR(builder->m_manip->m_em, "plugin_mask_block_builder_create_block: create block buf fail!");
        plugin_mask_data_block_free(block);
        return -1;
    }

    line_size = plugin_mask_data_block_buf_line_size(block);

    switch(plugin_mask_data_format(mask_data)) {
    case plugin_mask_data_format_bit:
        for(i = builder->m_lt_y; i < builder->m_rb_y; ++i, buf += line_size) {
            uint32_t const * lb = builder->m_buf + i * builder->m_width;
            for(j = builder->m_lt_x; j < builder->m_rb_x; ++j) {
                cpe_ba_set((cpe_ba_t)buf, j - builder->m_lt_x, lb[j] ? cpe_ba_true : cpe_ba_false);
            }
        }
        break;
    case plugin_mask_data_format_1:
        for(i = builder->m_lt_y; i < builder->m_rb_y; ++i, buf += line_size) {
            uint32_t const * lb = builder->m_buf + i * builder->m_width;
            for(j = builder->m_lt_x; j < builder->m_rb_x; ++j) {
                buf[j - builder->m_lt_x] = (uint8_t)lb[j];
            }
        }
        break;
    case plugin_mask_data_format_2:
        for(i = builder->m_lt_y; i < builder->m_rb_y; ++i, buf += line_size) {
            uint32_t const * lb = builder->m_buf + i * builder->m_width;
            for(j = builder->m_lt_x; j < builder->m_rb_x; ++j) {
                ((uint16_t *)buf)[j - builder->m_lt_x] = (uint16_t)lb[j];
            }
        }
        break;
    case plugin_mask_data_format_4:
        for(i = builder->m_lt_y; i < builder->m_rb_y; ++i, buf += line_size) {
            uint32_t const * lb = builder->m_buf + i * builder->m_width;
            for(j = builder->m_lt_x; j < builder->m_rb_x; ++j) {
                ((uint32_t *)buf)[j - builder->m_lt_x] = lb[j];
            }
        }
        break;
    }
    
    return 0;
}

int plugin_mask_block_builder_place_img_block(plugin_mask_block_builder_t builder, int32_t x, int32_t y, ui_data_img_block_t img_block) {
    ui_cache_res_t texture = ui_data_img_block_using_texture(img_block);
    ui_cache_pixel_buf_t pixel_buf;
    UI_IMG_BLOCK const * img_block_data = ui_data_img_block_data(img_block);
    uint32_t i, j;
    int32_t src_x;
    int32_t src_y;
    uint32_t src_w;
    uint32_t src_h;

    if (texture == NULL) {
        CPE_ERROR(
            builder->m_manip->m_em, "plugin_mask_block_builder_place_img_block: block %s no texture!",
            ui_data_img_block_name(img_block));
        return -1;
    }
    
    if (ui_cache_res_load_state(texture) != ui_cache_res_loaded) {
        if (ui_cache_res_load_sync(texture, NULL) != 0) {
            CPE_ERROR(
                builder->m_manip->m_em, "plugin_mask_block_builder_place_img_block: load texture %s fail!",
                ui_cache_res_path(texture));
            return -1;
        }
    }

    pixel_buf = ui_cache_texture_data_buf(texture);
    if (pixel_buf == NULL) {
        CPE_ERROR(
            builder->m_manip->m_em, "plugin_mask_block_builder_place_img_block: texture %s no pixel buf!",
            ui_cache_res_path(texture));
        return -1;
    }

    x -= builder->m_x;
    y -= builder->m_y;

    /*范围完全越界 */
    if ((x + (int32_t)img_block_data->src_w) < 0 || x >= (int32_t)builder->m_width
        || (y + (int32_t)img_block_data->src_h) < 0 || y >= (int32_t)builder->m_height) return 0;
        
    src_x = img_block_data->src_x;
    src_y = img_block_data->src_y;
    src_w = img_block_data->src_w;
    src_h = img_block_data->src_h;

    if (x < 0) {
        src_x += abs(x);
        src_w = src_w - abs(x);
        x = 0;
    }

    if (x + src_w > builder->m_width) {
        src_w = builder->m_width - x;
    }
    
    if (y < 0) {
        src_y += abs(y);
        src_h = src_y - abs(y);
        y = 0;
    }

    if (y + src_h > builder->m_height) {
        src_h = builder->m_height - y;
    }

    for(i = 0; i < src_h; ++i) {
        int32_t output_y;
        uint32_t * output_buf;

        output_y = y + i;
        assert(output_y >= 0 && output_y < builder->m_height);
        
        output_buf = builder->m_buf + (output_y * builder->m_width);
        
        for(j = 0; j < src_w; ++j) {
            assert((x + j) >= 0 && (x + j) < builder->m_width);

            output_buf[x + j] = ui_cache_pixel_buf_pixel_value_at(pixel_buf, 0, src_x + j, src_y + i, builder->m_source);
            //if (j < 90) printf("%c", output_buf[x + 1] ? '1' : '0');
        }
        //printf("\n");
    }

    if (!builder->m_have_data) {
        builder->m_lt_x = x;
        builder->m_lt_y = y;
        builder->m_rb_x = x + src_w;
        builder->m_rb_y = y + src_h;
    }
    else {
        if (x < builder->m_lt_x) builder->m_lt_x = x;
        if (y < builder->m_lt_y) builder->m_lt_y = y;
        if ((x + src_w) > builder->m_rb_x) builder->m_rb_x = x + src_w;
        if ((y + src_y) > builder->m_rb_y) builder->m_rb_y = y + src_h;
    }
    
    /* printf("xxxx: add block at (%d,%d), texture %p\n", x, y, texture); */
    /* printf("xxxx: texture=%s\n", ui_cache_res_path(texture)); */
    return 0;
}

int plugin_mask_block_builder_place_frame(plugin_mask_block_builder_t builder, int32_t x, int32_t y, ui_data_frame_t frame) {
    struct ui_data_frame_img_it img_it;
    ui_data_frame_img_t frame_img;
    int rv = 0;

    ui_data_frame_imgs(&img_it, frame);
    while((frame_img = ui_data_frame_img_it_next(&img_it))) {
        UI_IMG_REF const * img_ref_data = ui_data_frame_img_data(frame_img);
        ui_data_img_block_t img_block = ui_data_frame_img_using_img_block(frame_img);
        int32_t img_block_x = x;
        int32_t img_block_y = y;
        
        if (img_block == NULL) {
            CPE_ERROR(builder->m_manip->m_em, "plugin_mask_block_builder_place_frame: use img block not exist!");
            rv = -1;
            continue;
        }

        img_block_x += img_ref_data->trans.world_trans.trans.value[0];
        img_block_y += img_ref_data->trans.world_trans.trans.value[1];
        
        if (plugin_mask_block_builder_place_img_block(builder, img_block_x + x, img_block_y + y, img_block) != 0) rv = -1;
    }
    
    return rv;
}
