#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_bin_loader_i.h"

static int ui_data_sprite_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_bin_loader_t loader = ctx;
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_sprite_t sprite = ui_data_src_product(src);
    uint16_t frame_count = ui_data_sprite_frame_count(sprite);
    uint16_t frame_img_count = 0;
    uint16_t frame_collision_count = 0;    
    LPDRMETA frame_meta = ui_data_frame_meta(data_mgr);
    LPDRMETA frame_img_meta = ui_data_frame_img_meta(data_mgr);
    LPDRMETA frame_collision_meta = ui_data_frame_collision_meta(data_mgr);
    struct ui_data_frame_it frame_it;
    ui_data_frame_t frame;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    /*计算数据大小 */
    ui_data_sprite_frames(&frame_it, sprite);
    while((frame = ui_data_frame_it_next(&frame_it))) {
        frame_img_count += ui_data_frame_img_count(frame);
    }
    
    /*分配内存 */
    output_buf_len = (sizeof(UI_FRAME) * frame_count + sizeof(UI_IMG_REF) * frame_img_count) * 2;
    output_buf = mem_alloc(loader->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    Frames */
    CPE_COPY_HTON16(output_buf + total_sz, &frame_count);
    total_sz += 2;

    ui_data_sprite_frames(&frame_it, sprite);
    while((frame = ui_data_frame_it_next(&frame_it))) {
        UI_FRAME const * frame_data = ui_data_frame_data(frame);
        struct ui_data_frame_img_it frame_img_it;
        ui_data_frame_img_t frame_img;
        struct ui_data_frame_collision_it frame_collision_it;
        ui_data_frame_collision_t frame_collision;
    
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            frame_data, sizeof(*frame_data), frame_meta, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "load module: read frame data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        frame_img_count = ui_data_frame_img_count(frame);
        CPE_COPY_HTON16(output_buf + total_sz, &frame_img_count);
        total_sz += 2;

        ui_data_frame_imgs(&frame_img_it, frame);
        while((frame_img = ui_data_frame_img_it_next(&frame_img_it))) {
                UI_IMG_REF const * frame_img_data = ui_data_frame_img_data(frame_img);
    
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                frame_img_data, sizeof(*frame_img_data), frame_img_meta, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write sprite frame img data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }

        frame_collision_count = ui_data_frame_collision_count(frame);
        CPE_COPY_HTON16(output_buf + total_sz, &frame_collision_count);
        total_sz += 2;

        ui_data_frame_collisions(&frame_collision_it, frame);
        while((frame_collision = ui_data_frame_collision_it_next(&frame_collision_it))) {
            UI_COLLISION const * frame_collision_data = ui_data_frame_collision_data(frame_collision);
    
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                frame_collision_data, sizeof(*frame_collision_data), frame_collision_meta, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write sprite frame collision data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }
    
    /*写文件 */
    if (vfs_file_write(fp, output_buf, total_sz) != total_sz) {
        CPE_ERROR(em, "write to file fail!");
        goto COMPLETE;
    }

    rv = 0;

COMPLETE:
    if (output_buf) mem_free(loader->m_alloc, output_buf);
    return rv;
}

static int ui_data_sprite_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_sprite_t sprite = NULL;
    LPDRMETA frame_meta = ui_data_frame_meta(data_mgr);
    LPDRMETA frame_img_meta = ui_data_frame_img_meta(data_mgr);
    LPDRMETA frame_collision_meta = ui_data_frame_collision_meta(data_mgr);
    struct mem_buffer data_buff;
    uint16_t frame_count;
    uint16_t i, j;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len <= 0) {
        CPE_ERROR(em, "load sprite: read file fail!");
        goto COMPLETE;
    }
    input_data = mem_buffer_make_continuous(&data_buff, 0);
    
    sprite = ui_data_sprite_create(data_mgr, src);
    if (sprite == NULL) {
        CPE_ERROR(em, "load sprite: create sprite fail!");
        goto COMPLETE;
    }

    CPE_COPY_HTON16(&frame_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < frame_count; ++i) {
        ui_data_frame_t frame;
        UI_FRAME * frame_data;
        uint16_t frame_img_count;
        uint16_t frame_collision_count;

        frame = ui_data_frame_create(sprite);
        if (frame == NULL) {
            CPE_ERROR(em, "load sprite: create frame fail!");
            goto COMPLETE;
        }

        frame_data = ui_data_frame_data(frame);
        assert(frame_data);

        if (dr_pbuf_read_with_size(
                frame_data, sizeof(*frame_data),
                input_data + total_sz, data_len - total_sz, &input_used, frame_meta, em) < 0)
        {
            CPE_ERROR(em, "load sprite: read frame fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        if (frame_data->id != (uint32_t)-1) {
            uint32_t id = frame_data->id;
            frame_data->id = (uint32_t)-1;
            if (ui_data_frame_set_id(frame, id) != 0) {
                CPE_ERROR(em, "load module: set frame id %d fail!", id);
                goto COMPLETE;
            }
        }
        
        CPE_COPY_HTON16(&frame_img_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < frame_img_count; ++j) {
            ui_data_frame_img_t frame_img;
            UI_IMG_REF * frame_img_data;

            frame_img = ui_data_frame_img_create(frame);
            if (frame_img == NULL) {
                CPE_ERROR(em, "load sprite: create frame img fail!");
                goto COMPLETE;
            }

            frame_img_data = ui_data_frame_img_data(frame_img);
            assert(frame_img_data);

            if (dr_pbuf_read_with_size(
                    frame_img_data, sizeof(*frame_img_data),
                    input_data + total_sz, data_len - total_sz, &input_used, frame_img_meta, em) < 0)
            {
                CPE_ERROR(em, "load sprite: read frame img fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }

        CPE_COPY_HTON16(&frame_collision_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < frame_collision_count; ++j) {
            ui_data_frame_collision_t frame_collision;
            UI_COLLISION * frame_collision_data;

            frame_collision = ui_data_frame_collision_create(frame);
            if (frame_collision == NULL) {
                CPE_ERROR(em, "load sprite: create frame collision fail!");
                goto COMPLETE;
            }

            frame_collision_data = ui_data_frame_collision_data(frame_collision);
            assert(frame_collision_data);

            if (dr_pbuf_read_with_size(
                    frame_collision_data, sizeof(*frame_collision_data),
                    input_data + total_sz, data_len - total_sz, &input_used, frame_collision_meta, em) < 0)
            {
                CPE_ERROR(em, "load sprite: read frame collision fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }
    }

    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    return rv;
}

int ui_data_bin_save_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "frm.bin", ui_data_sprite_do_bin_save, ctx, em);
}

int ui_data_bin_load_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "frm.bin", ui_data_sprite_do_bin_load, ctx, em);
}

int ui_data_bin_rm_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "frm.bin", em);
}
