#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/mask/plugin_mask_module.h"
#include "plugin_mask_data_i.h"
#include "plugin_mask_data_block_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static int plugin_mask_data_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_mask_module_t module = ctx;
    plugin_mask_data_t data = ui_data_src_product(src);
    ui_string_table_t string_table = ui_data_src_strings(src);
    uint16_t block_count = data->m_block_count;
    uint8_t format = data->m_format;
    plugin_mask_data_block_t block;
    uint32_t total_buf_len;
    uint32_t output_buf_len;
    char * output_buf;
    int total_sz;

    if (string_table == NULL) {
        CPE_ERROR(em, " mask bin save: no strings!");
        return -1;
    }
    
    /*计算数据大小 */
    total_buf_len = 0;
    TAILQ_FOREACH(block, &data->m_blocks, m_next) {
        total_buf_len += block->m_buf_size;
    }
    
    /*分配内存 */
    output_buf_len =
        /*整体头部数据 */
        sizeof(block_count) + sizeof(format)
        /*block头部数据 */
        + (sizeof(uint32_t) * 9 * data->m_block_count)
        /*数据区域 */
        + total_buf_len
        /*名字表格 */
        + ui_string_table_data_size(string_table);
    
    output_buf = mem_buffer_alloc(gd_app_tmp_buffer(module->m_app), output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "save mask: alloc tmp buff fail!");
        return -1;
    }

    total_sz = 0;
    
    /*写数据 */
    memcpy(output_buf + total_sz, &format, sizeof(format));
    total_sz += sizeof(format);
    
    /*    blocks */
    CPE_COPY_HTON16(output_buf + total_sz, &block_count);
    total_sz += 2;
    TAILQ_FOREACH(block, &data->m_blocks, m_next) {
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_name); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_x); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_y); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_width); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_height); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_buf_x); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_buf_y); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_buf_width); total_sz += 4;
        CPE_COPY_HTON32(output_buf + total_sz, &block->m_buf_height); total_sz += 4;

        /*打印调试信息 */
        /* printf("xxxxx: write block:, pos=(%d,%d), size=(%d,%d), block-pos=(%d,%d), block-size=(%d,%d)\n", */
        /*        block->m_x, block->m_y, block->m_width, block->m_height, */
        /*        block->m_buf_x, block->m_buf_y, block->m_buf_width, block->m_buf_height); */
        /* if (block->m_buf_width < 200 && block->m_buf_height < 200) { */
        /*     for(uint32_t y = 0; y < block->m_buf_height; ++y) { */
        /*         char * line_start = ((char*)block->m_buf)  + y * cpe_ba_bytes_from_bits_m(block->m_buf_width); */
        /*         for(uint32_t x = 0; x < block->m_buf_width; ++x) { */
        /*             printf("%c", cpe_ba_get(line_start, x) ? '1' : '0'); */
        /*         } */
        /*         printf("\n"); */
        /*     } */
        /* } */
        
        if (block->m_buf) {
            memcpy(output_buf + total_sz, block->m_buf, block->m_buf_size);
        }
        else {
            bzero(output_buf + total_sz, block->m_buf_size);
        }
        total_sz += block->m_buf_size;
    }

    /*strings*/
    if (ui_string_table_data_size(string_table)) {
        assert(total_sz + ui_string_table_data_size(string_table) == output_buf_len);
        memcpy(output_buf + total_sz, ui_string_table_data(string_table), ui_string_table_data_size(string_table));
        total_sz += ui_string_table_data_size(string_table);
    }
    
    /*写文件 */
    if (vfs_file_write(fp, output_buf, total_sz) != total_sz) {
        CPE_ERROR(em, "write to file fail!");
        return -1;
    }

    return 0;
}

static int plugin_mask_data_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_mask_module_t module = ctx;
    plugin_mask_data_t data = NULL;
    mem_buffer_t buffer = gd_app_tmp_buffer(module->m_app);
    uint8_t format;
    uint16_t block_count;
    uint16_t i;
    ssize_t data_len;
    char * input_data;
    size_t input_used_size;

    mem_buffer_clear_data(buffer);
    data_len = vfs_file_load_to_buffer(buffer, fp);
    if (data_len <= 0) {
        CPE_ERROR(em, "load mask: read file fail!");
        return -1;
    }
    input_data = mem_buffer_make_continuous(buffer, 0);
    
    data = plugin_mask_data_create(module, src);
    if (data == NULL) {
        CPE_ERROR(em, "load mask: create sprite fail!");
        return -1;
    }

    input_used_size = 0;
    
    /*format*/
    format = input_data[0];
    input_used_size++;

    switch(format) {
    case plugin_mask_data_format_bit:
    case plugin_mask_data_format_1:
    case plugin_mask_data_format_2:
    case plugin_mask_data_format_4:
        break;
    default:
        CPE_ERROR(em, "load mask: format %d unknown!", format);
        plugin_mask_data_free(data);
        return -1;
    }
    plugin_mask_data_set_format(data, (plugin_mask_data_format_t)format);
    
    /*block*/
    CPE_COPY_HTON16(&block_count, input_data + input_used_size);
    input_used_size += 2;
    for(i = 0; i < block_count; ++i) {
        plugin_mask_data_block_t block;
        uint32_t name;
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
        uint32_t buf_x;
        uint32_t buf_y;
        uint32_t buf_width;
        uint32_t buf_height;
        void * buf;

        CPE_COPY_HTON32(&name, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&x, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&y, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&width, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&height, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&buf_x, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&buf_y, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&buf_width, input_data + input_used_size); input_used_size += 4;
        CPE_COPY_HTON32(&buf_height, input_data + input_used_size); input_used_size += 4;

        /* printf("xxxxx: load block:, pos=(%d,%d), size=(%d,%d), block-pos=(%d,%d), block-size=(%d,%d)\n", */
        /*        x, y, width, height, buf_x, buf_y, buf_width, buf_height); */
            
        block = plugin_mask_data_block_create(data, name, x, y, width, height, buf_x, buf_y, buf_width, buf_height);
        if (block == NULL) {
            CPE_ERROR(em, "load mask: create block fail!");
            plugin_mask_data_free(data);
            return -1;
        }

        buf = plugin_mask_data_block_check_create_buf(block);
        if (buf == NULL) {
            CPE_ERROR(em, "load mask: block check create buf fail!");
            plugin_mask_data_free(data);
            return -1;
        }

        memcpy(buf, input_data + input_used_size, block->m_buf_size);
        input_used_size += block->m_buf_size;
    }

    if (input_used_size >= data_len) {
        CPE_ERROR(em, "load mask: no string table data!");
        plugin_mask_data_free(data);
        return -1;
    }

    if (ui_data_src_strings_set(src, input_data + input_used_size, data_len - input_used_size) != 0) {
        CPE_ERROR(em, "load mask: load string table fail!");
        plugin_mask_data_free(data);
        return -1;
    }
    
    return 0;
}

int plugin_mask_data_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "mask.bin", plugin_mask_data_do_bin_save, ctx, em);
}

int plugin_mask_data_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "mask.bin", plugin_mask_data_do_bin_load, ctx, em);
}

int plugin_mask_data_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "mask.bin", em);
}

#ifdef __cplusplus
}
#endif
