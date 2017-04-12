#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/model/ui_data_action.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_bin_loader_i.h"

static int ui_data_action_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_bin_loader_t loader = ctx;
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_action_t action = ui_data_src_product(src);
    uint16_t actor_count = ui_data_action_actor_count(action);
    uint16_t actor_layer_count = 0;
    uint16_t actor_frame_count = 0;
    LPDRMETA actor_meta = ui_data_actor_meta(data_mgr);
    LPDRMETA actor_layer_meta = ui_data_actor_layer_meta(data_mgr);
    LPDRMETA actor_frame_meta = ui_data_actor_frame_meta(data_mgr);
    struct ui_data_actor_it actor_it;
    struct ui_data_actor_layer_it actor_layer_it;
    struct ui_data_actor_frame_it actor_frame_it;
    ui_data_actor_t actor;
    ui_data_actor_layer_t actor_layer;
    ui_data_actor_frame_t actor_frame;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    /*计算数据大小 */
    ui_data_action_actors(&actor_it, action);
    while((actor = ui_data_actor_it_next(&actor_it))) {
        actor_layer_count += ui_data_actor_layer_count(actor);

        ui_data_actor_layers(&actor_layer_it, actor);
        while((actor_layer = ui_data_actor_layer_it_next(&actor_layer_it))) {
            actor_frame_count += ui_data_actor_layer_frame_count(actor_layer);
        }
    }
    
    /*分配内存 */
    output_buf_len = (sizeof(UI_ACTOR) * actor_count
                      + sizeof(UI_ACTOR_LAYER) * actor_layer_count
                      + sizeof(UI_ACTOR_FRAME) * actor_frame_count) * 2;
    output_buf = mem_alloc(loader->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    Frames */
    CPE_COPY_HTON16(output_buf + total_sz, &actor_count);
    total_sz += 2;

    ui_data_action_actors(&actor_it, action);
    while((actor = ui_data_actor_it_next(&actor_it))) {
        UI_ACTOR const * actor_data = ui_data_actor_data(actor);
    
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            actor_data, sizeof(*actor_data), actor_meta, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "write action actor data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        actor_layer_count = ui_data_actor_layer_count(actor);
        CPE_COPY_HTON16(output_buf + total_sz, &actor_layer_count);
        total_sz += 2;

        ui_data_actor_layers(&actor_layer_it, actor);
        while((actor_layer = ui_data_actor_layer_it_next(&actor_layer_it))) {
            UI_ACTOR_LAYER const * actor_layer_data = ui_data_actor_layer_data(actor_layer);
    
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                actor_layer_data, sizeof(*actor_layer_data), actor_layer_meta, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write action actor layer data fail!");
                goto COMPLETE;
            }
            total_sz += sz;

            actor_frame_count = ui_data_actor_layer_frame_count(actor_layer);
            CPE_COPY_HTON16(output_buf + total_sz, &actor_frame_count);
            total_sz += 2;

            ui_data_actor_layer_frames(&actor_frame_it, actor_layer);
            while((actor_frame = ui_data_actor_frame_it_next(&actor_frame_it))) {
                UI_ACTOR_FRAME const * actor_frame_data = ui_data_actor_frame_data(actor_frame);
    
                sz = dr_pbuf_write_with_size(
                    output_buf + total_sz, output_buf_len - total_sz,
                    actor_frame_data, sizeof(*actor_frame_data), actor_frame_meta, NULL);
                if (sz < 0) {
                    CPE_ERROR(em, "write action actor frame data fail!");
                    goto COMPLETE;
                }
                total_sz += sz;
            }
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

static int ui_data_action_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_action_t action = NULL;
    LPDRMETA actor_meta = ui_data_actor_meta(data_mgr);
    LPDRMETA actor_layer_meta = ui_data_actor_layer_meta(data_mgr);
    LPDRMETA actor_frame_meta = ui_data_actor_frame_meta(data_mgr);
    struct mem_buffer data_buff;
    uint16_t actor_count;
    uint16_t actor_layer_count;
    uint16_t actor_frame_count;
    uint16_t i, j, k;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len <= 0) {
        CPE_ERROR(em, "load action: read file fail!");
        goto COMPLETE;
    }
    input_data = mem_buffer_make_continuous(&data_buff, 0);
    
    action = ui_data_action_create(data_mgr, src);
    if (action == NULL) {
        CPE_ERROR(em, "load action: create action fail!");
        goto COMPLETE;
    }

    CPE_COPY_HTON16(&actor_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < actor_count; ++i) {
        ui_data_actor_t actor;
        UI_ACTOR * actor_data;

        actor = ui_data_actor_create(action);
        if (actor == NULL) {
            CPE_ERROR(em, "load action: create actor fail!");
            goto COMPLETE;
        }

        actor_data = ui_data_actor_data(actor);
        assert(actor_data);

        if (dr_pbuf_read_with_size(
                actor_data, sizeof(*actor_data),
                input_data + total_sz, data_len - total_sz, &input_used, actor_meta, em) < 0)
        {
            CPE_ERROR(em, "load action: read actor fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        if (actor_data->id != (uint32_t)-1) {
            uint32_t id = actor_data->id;
            actor_data->id = (uint32_t)-1;
            if (ui_data_actor_set_id(actor, id) != 0) {
                CPE_ERROR(em, "load module: set actor id %d fail!", id);
                goto COMPLETE;
            }
        }

        CPE_COPY_HTON16(&actor_layer_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < actor_layer_count; ++j) {
            ui_data_actor_layer_t actor_layer;
            UI_ACTOR_LAYER * actor_layer_data;

            actor_layer = ui_data_actor_layer_create(actor);
            if (actor_layer == NULL) {
                CPE_ERROR(em, "load action: create actor layer fail!");
                goto COMPLETE;
            }

            actor_layer_data = ui_data_actor_layer_data(actor_layer);
            assert(actor_layer_data);

            if (dr_pbuf_read_with_size(
                    actor_layer_data, sizeof(*actor_layer_data),
                    input_data + total_sz, data_len - total_sz, &input_used, actor_layer_meta, em) < 0)
            {
                CPE_ERROR(em, "load action: read frame img fail!");
                goto COMPLETE;
            }
            total_sz += input_used;

            CPE_COPY_HTON16(&actor_frame_count, input_data + total_sz);
            total_sz += 2;

            for(k = 0; k < actor_frame_count; ++k) {
                ui_data_actor_frame_t actor_frame;
                UI_ACTOR_FRAME * actor_frame_data;

                actor_frame = ui_data_actor_frame_create(actor_layer);
                if (actor_frame == NULL) {
                    CPE_ERROR(em, "load action: create actor frame fail!");
                    goto COMPLETE;
                }

                actor_frame_data = ui_data_actor_frame_data(actor_frame);
                assert(actor_frame_data);

                if (dr_pbuf_read_with_size(
                        actor_frame_data, sizeof(*actor_frame_data),
                        input_data + total_sz, data_len - total_sz, &input_used, actor_frame_meta, em) < 0)
                {
                    CPE_ERROR(em, "load action: read frame img fail!");
                    goto COMPLETE;
                }
                total_sz += input_used;
            }
        }
    }

    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    return rv;
}

int ui_data_bin_save_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "act.bin", ui_data_action_do_bin_save, ctx, em);
}

int ui_data_bin_load_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "act.bin", ui_data_action_do_bin_load, ctx, em);
}

int ui_data_bin_rm_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "act.bin", em);
}
