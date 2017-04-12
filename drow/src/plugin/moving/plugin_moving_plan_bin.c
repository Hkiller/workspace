#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/moving/plugin_moving_module.h"
#include "plugin_moving_plan_i.h"
#include "plugin_moving_plan_track_i.h"
#include "plugin_moving_plan_point_i.h"
#include "plugin_moving_plan_node_i.h"
#include "plugin_moving_plan_segment_i.h"

static int plugin_moving_plan_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_moving_module_t module = ctx;
    plugin_moving_plan_t plan = ui_data_src_product(src);
    uint16_t point_count = 0;
    uint16_t segment_count = 0;
    struct plugin_moving_plan_track_it track_it;
    plugin_moving_plan_track_t track;
    struct plugin_moving_plan_node_it node_it;
    plugin_moving_plan_node_t node;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    /*计算数据大小 */
    plugin_moving_plan_tracks(&track_it, plan);
    while((track = plugin_moving_plan_track_it_next(&track_it))) {
        point_count += track->m_point_count;
    }
    
    plugin_moving_plan_nodes(&node_it, plan);
    while((node = plugin_moving_plan_node_it_next(&node_it))) {
        segment_count += node->m_segment_count;
    }

    /*分配内存 */
    output_buf_len = (sizeof(MOVING_PLAN_TRACK) * plan->m_track_count
                      + sizeof(MOVING_PLAN_POINT) * point_count
                      + sizeof(MOVING_PLAN_NODE) * plan->m_node_count
                      + sizeof(MOVING_PLAN_SEGMENT) * segment_count) * 2;
    output_buf = mem_alloc(module->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    头数据 */
    sz = dr_pbuf_write_with_size(
        output_buf + total_sz, output_buf_len - total_sz,
        &plan->m_data, sizeof(plan->m_data), module->m_meta_moving_plan, NULL);
    if (sz < 0) {
        CPE_ERROR(em, "write plan data fail!");
        goto COMPLETE;
    }
    total_sz += sz;

    /*    Tracks */
    CPE_COPY_HTON16(output_buf + total_sz, &plan->m_track_count);
    total_sz += 2;

    plugin_moving_plan_tracks(&track_it, plan);
    while((track = plugin_moving_plan_track_it_next(&track_it))) {
        struct plugin_moving_plan_point_it point_it;
        plugin_moving_plan_point_t point;

        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            &track->m_data, sizeof(track->m_data), module->m_meta_moving_plan_track, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "write plan track data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        CPE_COPY_HTON16(output_buf + total_sz, &track->m_point_count);
        total_sz += 2;
        
        plugin_moving_plan_track_points(&point_it, track);
        while((point = plugin_moving_plan_point_it_next(&point_it))) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                &point->m_data, sizeof(point->m_data), module->m_meta_moving_plan_point, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write plan point data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }
    
    /*    Nodes */
    CPE_COPY_HTON16(output_buf + total_sz, &plan->m_node_count);
    total_sz += 2;
    
    plugin_moving_plan_nodes(&node_it, plan);
    while((node = plugin_moving_plan_node_it_next(&node_it))) {
        struct plugin_moving_plan_segment_it segment_it;
        plugin_moving_plan_segment_t segment;

        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            &node->m_data, sizeof(node->m_data), module->m_meta_moving_plan_node, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "write plan node data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        CPE_COPY_HTON16(output_buf + total_sz, &node->m_segment_count);
        total_sz += 2;
        
        plugin_moving_plan_node_segments(&segment_it, node);
        while((segment = plugin_moving_plan_segment_it_next(&segment_it))) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                &segment->m_data, sizeof(segment->m_data), module->m_meta_moving_plan_segment, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write plan segment data fail!");
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

static int plugin_moving_plan_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_moving_module_t module = ctx;
    plugin_moving_plan_t plan = NULL;
    MOVING_PLAN * plan_data;
    struct mem_buffer data_buff;
    ssize_t data_len;
    char * input_data;
    uint16_t track_count;
    uint16_t node_count;
    uint16_t i, j;
    int rv = -1;
    size_t input_used;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len < 0) {
        CPE_ERROR(em, "load moving plan: read file fail!");
        goto COMPLETE;
    } 
    input_data = mem_buffer_make_continuous(&data_buff, 0);

    plan = plugin_moving_plan_create(module, src);
    if (plan == NULL) {
        CPE_ERROR(em, "load moving plan: read file fail!");
        goto COMPLETE;
    }
    plan_data = plugin_moving_plan_data(plan);
    
    if (dr_pbuf_read_with_size(plan_data, sizeof(*plan_data), input_data, data_len, &input_used, module->m_meta_moving_plan, em) < 0) {
        CPE_ERROR(em, "load moving plan: read record fail!");
        goto COMPLETE;
    }
    total_sz += input_used;

    CPE_COPY_HTON16(&track_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < track_count; ++i) {
        plugin_moving_plan_track_t track;
        MOVING_PLAN_TRACK * track_data;
        uint16_t point_count;
        
        track = plugin_moving_plan_track_create(plan);
        if (track == NULL) {
            CPE_ERROR(em, "load moving plan: create emitter track fail!");
            goto COMPLETE;
        }

        track_data = plugin_moving_plan_track_data(track);
        assert(track_data);

        if (dr_pbuf_read_with_size(
                track_data, sizeof(*track_data),
                input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_moving_plan_track, em) < 0)
        {
            CPE_ERROR(em, "load moving plan: read record fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        CPE_COPY_HTON16(&point_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < point_count; ++j) {
            plugin_moving_plan_point_t point;
            MOVING_PLAN_POINT * point_data;
        
            point = plugin_moving_plan_point_create(track);
            if (point == NULL) {
                CPE_ERROR(em, "load moving plan: create point fail!");
                goto COMPLETE;
            }

            point_data = plugin_moving_plan_point_data(point);
            assert(point_data);

            if (dr_pbuf_read_with_size(
                    point_data, sizeof(*point_data),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_moving_plan_point, em) < 0)
            {
                CPE_ERROR(em, "load moving plan: read point fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }
    }
    
    CPE_COPY_HTON16(&node_count, input_data + total_sz);
    total_sz += 2;
    
    for(i = 0; i < node_count; ++i) {
        plugin_moving_plan_node_t node;
        MOVING_PLAN_NODE * node_data;
        uint16_t segment_count;
        
        node = plugin_moving_plan_node_create(plan);
        if (node == NULL) {
            CPE_ERROR(em, "load moving plan: create emitter node fail!");
            goto COMPLETE;
        }

        node_data = plugin_moving_plan_node_data(node);
        assert(node_data);

        if (dr_pbuf_read_with_size(
                node_data, sizeof(*node_data),
                input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_moving_plan_node, em) < 0)
        {
            CPE_ERROR(em, "load moving plan: read record fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        CPE_COPY_HTON16(&segment_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < segment_count; ++j) {
            plugin_moving_plan_segment_t segment;
            MOVING_PLAN_SEGMENT * segment_data;
        
            segment = plugin_moving_plan_segment_create(node);
            if (segment == NULL) {
                CPE_ERROR(em, "load moving plan: create segment fail!");
                goto COMPLETE;
            }

            segment_data = plugin_moving_plan_segment_data(segment);
            assert(segment_data);

            if (dr_pbuf_read_with_size(
                    segment_data, sizeof(*segment_data),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_moving_plan_segment, em) < 0)
            {
                CPE_ERROR(em, "load moving plan: read segment fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }
    }

    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    if (rv != 0 && plan) {
        plugin_moving_plan_free(plan);
    }

    return rv;
}

int plugin_moving_plan_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "moving.bin", plugin_moving_plan_do_bin_save, ctx, em);
}

int plugin_moving_plan_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "moving.bin", plugin_moving_plan_do_bin_load, ctx, em);
}

int plugin_moving_plan_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "moving.bin", em);
}
