#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/scrollmap/plugin_scrollmap_module.h"
#include "plugin_scrollmap_data_scene_i.h"
#include "plugin_scrollmap_data_tile_i.h"
#include "plugin_scrollmap_data_layer_i.h"
#include "plugin_scrollmap_data_block_i.h"
#include "plugin_scrollmap_data_script_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static int plugin_scrollmap_data_scene_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_scrollmap_module_t module = ctx;
    plugin_scrollmap_data_scene_t scene = ui_data_src_product(src);
    uint16_t tile_count = scene->m_tile_count;
    uint16_t layer_count = scene->m_layer_count;
    uint16_t block_count = 0;
    uint16_t script_count = 0;    
    plugin_scrollmap_data_tile_t tile;
    plugin_scrollmap_data_layer_t layer;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    /*计算数据大小 */
    TAILQ_FOREACH(layer, &scene->m_layers, m_next) {
        block_count += layer->m_block_count;
        script_count += layer->m_script_count;
    }
    
    /*分配内存 */
    output_buf_len = (sizeof(SCROLLMAP_TILE) * tile_count
                      + sizeof(SCROLLMAP_LAYER) * layer_count
                      + sizeof(SCROLLMAP_BLOCK) * block_count
                      + sizeof(SCROLLMAP_SCRIPT) * script_count) * 2;
    output_buf = mem_alloc(module->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "save scrollmap: alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    tiles */
    CPE_COPY_HTON16(output_buf + total_sz, &tile_count);
    total_sz += 2;
    TAILQ_FOREACH(tile, &scene->m_tiles, m_next) {
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            &tile->m_data, sizeof(tile->m_data), module->m_meta_tile, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "save scrollmap: write tile data fail!");
            goto COMPLETE;
        }
        total_sz += sz;
    }
    
    /*    layer */
    CPE_COPY_HTON16(output_buf + total_sz, &layer_count);
    total_sz += 2;
    TAILQ_FOREACH(layer, &scene->m_layers, m_next) {
        plugin_scrollmap_data_block_t block;
        plugin_scrollmap_data_script_t script;
    
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            &layer->m_data, sizeof(layer->m_data), module->m_meta_layer, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "save scrollmap: write layer data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        /*block*/
        block_count = layer->m_block_count;
        CPE_COPY_HTON16(output_buf + total_sz, &block_count);
        total_sz += 2;

        TAILQ_FOREACH(block, &layer->m_blocks, m_next) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                &block->m_data, sizeof(block->m_data), module->m_meta_block, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "save scrollmap: write block fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }

        /*script */
        script_count = layer->m_script_count;
        CPE_COPY_HTON16(output_buf + total_sz, &script_count);
        total_sz += 2;

        TAILQ_FOREACH(script, &layer->m_scripts, m_next) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                &script->m_data, sizeof(script->m_data), module->m_meta_script, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "save scrollmap: write script fail!");
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
    if (output_buf) mem_free(module->m_alloc, output_buf);
    return rv;
}

static int plugin_scrollmap_data_scene_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_scrollmap_module_t module = ctx;
    plugin_scrollmap_data_scene_t scene = NULL;
    struct mem_buffer data_buff;
    uint16_t tile_count;
    uint16_t layer_count;
    uint16_t i, j;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len <= 0) {
        CPE_ERROR(em, "load scrollmap: read file fail!");
        goto COMPLETE;
    }
    input_data = mem_buffer_make_continuous(&data_buff, 0);
    
    scene = plugin_scrollmap_data_scene_create(module, src);
    if (scene == NULL) {
        CPE_ERROR(em, "load scrollmap: create sprite fail!");
        goto COMPLETE;
    }

    /*tile*/
    CPE_COPY_HTON16(&tile_count, input_data + total_sz);
    total_sz += 2;
    for(i = 0; i < tile_count; ++i) {
        plugin_scrollmap_data_tile_t tile;

        tile = plugin_scrollmap_data_tile_create(scene);;
        if (tile == NULL) {
            CPE_ERROR(em, "load scrollmap: create tile fail!");
            goto COMPLETE;
        }

        if (dr_pbuf_read_with_size(
                &tile->m_data, sizeof(tile->m_data),
                input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_tile, em) < 0)
        {
            CPE_ERROR(em, "load scrollmap: read tile fail!");
            goto COMPLETE;
        }
        total_sz += input_used;
    }

    /*layer*/
    CPE_COPY_HTON16(&layer_count, input_data + total_sz);
    total_sz += 2;
    for(i = 0; i < layer_count; ++i) {
        plugin_scrollmap_data_layer_t layer;
        uint16_t block_count;
        uint16_t script_count;

        layer = plugin_scrollmap_data_layer_create(scene);
        if (layer == NULL) {
            CPE_ERROR(em, "load scrollmap: create layer fail!");
            goto COMPLETE;
        }

        if (dr_pbuf_read_with_size(
                &layer->m_data, sizeof(layer->m_data),
                input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_layer, em) < 0)
        {
            CPE_ERROR(em, "load scrollmap: read layer data fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        /*block*/
        CPE_COPY_HTON16(&block_count, input_data + total_sz);
        total_sz += 2;
        for(j = 0; j < block_count; ++j) {
            plugin_scrollmap_data_block_t block;

            block = plugin_scrollmap_data_block_create(layer);
            if (block == NULL) {
                CPE_ERROR(em, "load scrollmap: create block fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    &block->m_data, sizeof(block->m_data),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_block, em) < 0)
            {
                CPE_ERROR(em, "load scrollmap: read block fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }

        /*script*/
        CPE_COPY_HTON16(&script_count, input_data + total_sz);
        total_sz += 2;
        for(j = 0; j < script_count; ++j) {
            plugin_scrollmap_data_script_t script;

            script = plugin_scrollmap_data_script_create(layer);
            if (script == NULL) {
                CPE_ERROR(em, "load scrollmap: create script fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    &script->m_data, sizeof(script->m_data),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_script, em) < 0)
            {
                CPE_ERROR(em, "load scrollmap: read script fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }
    }

    //printf("xxxxx: dump\n%s", plugin_scrollmap_data_scene_dump(&data_buff, scene));
    
    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    return rv;
}

int plugin_scrollmap_data_scene_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "scrollmap.bin", plugin_scrollmap_data_scene_do_bin_save, ctx, em);
}

int plugin_scrollmap_data_scene_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "scrollmap.bin", plugin_scrollmap_data_scene_do_bin_load, ctx, em);
}

int plugin_scrollmap_data_scene_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "scrollmap.bin", em);
}

#ifdef __cplusplus
}
#endif
