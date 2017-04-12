#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"
#include "plugin_tiledmap_data_scene_i.h"
#include "plugin_tiledmap_data_layer_i.h"
#include "plugin_tiledmap_data_tile_i.h"

static int plugin_tiledmap_data_scene_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_tiledmap_module_t module = (plugin_tiledmap_module_t)ctx;
    plugin_tiledmap_data_scene_t scene = (plugin_tiledmap_data_scene_t)ui_data_src_product(src);
    plugin_tiledmap_data_layer_t layer;
    plugin_tiledmap_data_tile_t tile;
    LPDRMETA scene_meta = plugin_tiledmap_module_data_scene_meta(module);
    LPDRMETA layer_meta = plugin_tiledmap_module_data_layer_meta(module);
    LPDRMETA tile_meta = plugin_tiledmap_module_data_tile_meta(module);
    char * output_buf = NULL;
    size_t output_sz;
    int sz;
    int total_sz;
    int rv = -1;

    /*alloc buf*/
    output_sz = sizeof(scene->m_data) + sizeof(TILEDMAP_LAYER) * scene->m_layer_count;
    TAILQ_FOREACH(layer, &scene->m_layer_list, m_next_for_scene) {
        output_sz += sizeof(TILEDMAP_TILE) * layer->m_tile_count;
    }
    output_sz *= 2;
        
    output_buf = (char*)mem_alloc(module->m_alloc, output_sz);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc output buf fail, size=%d!", (int)output_sz);
        goto COMPLETE;
    }

    /*begin write data*/
    total_sz = 0;
    
    /*scene*/
    sz = dr_pbuf_write_with_size(output_buf, output_sz, &scene->m_data, sizeof(scene->m_data), scene_meta, em);
    if (sz < 0) {
        CPE_ERROR(em, "pbuf write fail!");
        goto COMPLETE;
    }
    total_sz += sz;

    /*layer*/
    CPE_COPY_HTON16(output_buf + total_sz, &scene->m_layer_count);
    total_sz += 2;
    
    TAILQ_FOREACH(layer, &scene->m_layer_list, m_next_for_scene) {
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_sz - total_sz,
            &layer->m_data, sizeof(layer->m_data), layer_meta, em);
        if (sz < 0) {
            CPE_ERROR(em, "pbuf write layer fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        CPE_COPY_HTON16(output_buf + total_sz, &layer->m_tile_count);
        total_sz += 2;

        /*tile*/        
        TAILQ_FOREACH(tile, &layer->m_tile_list, m_next_for_layer) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_sz - total_sz,
                &tile->m_data, sizeof(tile->m_data), tile_meta, em);
            if (sz < 0) {
                CPE_ERROR(em, "pbuf write tile fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }

    /*write to file*/
    if (vfs_file_write(fp, output_buf, total_sz) != total_sz) {
        CPE_ERROR(em, "write to file fail!");
        goto COMPLETE;
    }

    rv = 0;

COMPLETE:
    if (output_buf) mem_free(module->m_alloc, output_buf);

    return rv;
}

static int plugin_tiledmap_data_scene_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_tiledmap_module_t module = (plugin_tiledmap_module_t)ctx;
    plugin_tiledmap_data_scene_t scene = NULL;
    plugin_tiledmap_data_layer_t layer;
    plugin_tiledmap_data_tile_t tile;
    LPDRMETA scene_meta = plugin_tiledmap_module_data_scene_meta(module);
    LPDRMETA layer_meta = plugin_tiledmap_module_data_layer_meta(module);
    LPDRMETA tile_meta = plugin_tiledmap_module_data_tile_meta(module);
    struct mem_buffer data_buff;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    uint16_t layer_count;
    uint16_t tile_count;
    uint16_t layer_i;
    uint16_t tile_i;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len < 0) {
        CPE_ERROR(em, "load tiledmap: read file fail!");
        goto COMPLETE;
    } 
    input_data = (char*)mem_buffer_make_continuous(&data_buff, 0);

    scene = plugin_tiledmap_data_scene_create(module, src);
    if (scene == NULL) {
        CPE_ERROR(em, "load tiledmap: create scene fail!");
        goto COMPLETE;
    }

    /*scene*/
    if (dr_pbuf_read_with_size(&scene->m_data, sizeof(scene->m_data), input_data, data_len, &input_used, scene_meta, em) < 0) {
        CPE_ERROR(em, "load tiledmap: read scene fail!");
        goto COMPLETE;
    }
    total_sz += input_used;

    /*layer*/
    CPE_COPY_HTON16(&layer_count, input_data + total_sz);
    total_sz += 2;

    for(layer_i = 0; layer_i < layer_count; ++layer_i) {
        layer = plugin_tiledmap_data_layer_create(scene);
        if (layer == NULL) {
            CPE_ERROR(em, "load tiledmap: create layer fail!");
            goto COMPLETE;
        }

        if (dr_pbuf_read_with_size(
                &layer->m_data, sizeof(layer->m_data),
                input_data + total_sz, data_len - total_sz, &input_used, layer_meta, em) < 0)
        {
            CPE_ERROR(em, "load tiledmap: read layer fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        /*tile*/
        CPE_COPY_HTON16(&tile_count, input_data + total_sz);
        total_sz += 2;

        for(tile_i = 0; tile_i < tile_count; ++tile_i) {
            tile = plugin_tiledmap_data_tile_create(layer);
            if (tile == NULL) {
                CPE_ERROR(em, "load tiledmap: create tile fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    &tile->m_data, sizeof(tile->m_data),
                    input_data + total_sz, data_len - total_sz, &input_used, tile_meta, em) < 0)
            {
                CPE_ERROR(em, "load tiledmap: read tile fail!");
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

int plugin_tiledmap_data_scene_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "tiledmap.bin", plugin_tiledmap_data_scene_do_bin_save, ctx, em);
}

int plugin_tiledmap_data_scene_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "tiledmap.bin", plugin_tiledmap_data_scene_do_bin_load, ctx, em);
}

int plugin_tiledmap_data_scene_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "tiledmap.bin", em);
}
