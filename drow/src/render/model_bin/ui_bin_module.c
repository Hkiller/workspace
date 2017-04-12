#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_bin_loader_i.h"

static int ui_data_module_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_bin_loader_t loader = ctx;
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_module_t module = ui_data_src_product(src);
    ui_string_table_t string_table = ui_data_src_strings(src);
    uint16_t img_block_count;
    LPDRMETA img_block_meta = ui_data_img_block_meta(data_mgr);
    struct ui_data_img_block_it img_block_it;
    ui_data_img_block_t img_block;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    if (string_table == NULL) {
        CPE_ERROR(em, " module bin save: no strings!");
        goto COMPLETE;
    }
    
    /*计算数据大小 */
    img_block_count = 0;
    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        img_block_count++;
    }
    
    /*分配内存 */
    output_buf_len = (sizeof(UI_IMG_BLOCK) * img_block_count) * 2 + ui_string_table_data_size(string_table);
    output_buf = mem_alloc(loader->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    Tracks */
    CPE_COPY_HTON16(output_buf + total_sz, &img_block_count);
    total_sz += 2;

    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        UI_IMG_BLOCK const * img_block_data = ui_data_img_block_data(img_block);
        
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            img_block_data, sizeof(*img_block_data), img_block_meta, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "write plan img block data fail!");
            goto COMPLETE;
        }
        total_sz += sz;
    }

    /*strings*/
    if (ui_string_table_data_size(string_table)) {
        if (total_sz + ui_string_table_data_size(string_table) > output_buf_len) {
            CPE_ERROR(
                em, "save module: write string table not enough output, size=%d, but only %d!",
                (int)ui_string_table_data_size(string_table), (int)(output_buf_len - total_sz));
            goto COMPLETE;
        }

        memcpy(output_buf + total_sz, ui_string_table_data(string_table), ui_string_table_data_size(string_table));
        total_sz += ui_string_table_data_size(string_table);
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

static int ui_data_module_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_module_t module = NULL;
    LPDRMETA img_block_meta = ui_data_img_block_meta(data_mgr);
    struct mem_buffer data_buff;
    uint16_t img_block_count;
    uint16_t i;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len < 0) {
        CPE_ERROR(em, "load module: read file fail!");
        goto COMPLETE;
    } 
    input_data = mem_buffer_make_continuous(&data_buff, 0);
    
    module = ui_data_module_create(data_mgr, src);
    if (module == NULL) {
        CPE_ERROR(em, "load module: create module fail!");
        goto COMPLETE;
    }

    CPE_COPY_HTON16(&img_block_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < img_block_count; ++i) {
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK * img_block_data;

        img_block = ui_data_img_block_create(module);
        if (img_block == NULL) {
            CPE_ERROR(em, "load module: create img block fail!");
            goto COMPLETE;
        }

        img_block_data = ui_data_img_block_data(img_block);
        assert(img_block_data);

        if (dr_pbuf_read_with_size(
                img_block_data, sizeof(*img_block_data),
                input_data + total_sz, data_len - total_sz, &input_used, img_block_meta, em) < 0)
        {
            CPE_ERROR(em, "load module: read img block!");
            goto COMPLETE;
        }
        total_sz += input_used;

        if (img_block_data->id != (uint32_t)-1) {
            uint32_t id = img_block_data->id;
            img_block_data->id = (uint32_t)-1;
            if (ui_data_img_block_set_id(img_block, id) != 0) {
                CPE_ERROR(em, "load module: set img block id %d fail!", id);
                goto COMPLETE;
            }
        }
    }

    if (total_sz >= data_len) {
        CPE_ERROR(em, "load particle: no string table data!");
        goto COMPLETE;
    }

    if (ui_data_src_strings_set(src, input_data + total_sz, data_len - total_sz) != 0) {
        CPE_ERROR(em, "load particle: load string table fail!");
        goto COMPLETE;
    }
    
    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    return rv;
}

int ui_data_bin_save_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "ibk.bin", ui_data_module_do_bin_save, ctx, em);
}

int ui_data_bin_load_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "ibk.bin", ui_data_module_do_bin_load, ctx, em);
}

int ui_data_bin_rm_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "ibk.bin", em);
}
