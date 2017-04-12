#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/spine/plugin_spine_data_state_def.h"
#include "plugin/spine/plugin_spine_atlas.h"
#include "render/model/ui_data_src.h"
#include "plugin_spine_data_state_def_i.h"

static int plugin_spine_data_state_def_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_spine_module_t module = ctx;
    plugin_spine_data_state_def_t def = ui_data_src_product(src);
    ui_string_table_t string_table = ui_data_src_strings(src);
    uint16_t part_count = def->m_part_count;
    uint16_t part_state_count = 0;
    uint16_t part_transition_count = 0;
    plugin_spine_data_part_t part;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    if (string_table == NULL) {
        CPE_ERROR(em, " spine-state bin save: no strings!");
        goto COMPLETE;
    }
    
    /*计算数据大小 */
    TAILQ_FOREACH(part, &def->m_parts, m_next) {
        part_state_count += part->m_state_count;
        part_transition_count += part->m_transition_count;
    }
    
    /*分配内存 */
    output_buf_len = (sizeof(SPINE_PART) * part_count
                      + sizeof(SPINE_PART_STATE) * part_state_count
                      + sizeof(SPINE_PART_TRANSITION) * part_transition_count) * 2
        + ui_string_table_data_size(string_table);
    output_buf = mem_alloc(module->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    Frames */
    CPE_COPY_HTON16(output_buf + total_sz, &part_count);
    total_sz += 2;

    TAILQ_FOREACH(part, &def->m_parts, m_next) {
        plugin_spine_data_part_state_t state;
        plugin_spine_data_part_transition_t transition;
    
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            &part->m_data, sizeof(part->m_data), module->m_meta_data_part, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "load module: read part data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        part_state_count = part->m_state_count;
        CPE_COPY_HTON16(output_buf + total_sz, &part_state_count);
        total_sz += 2;

        TAILQ_FOREACH(state, &part->m_states, m_next) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                &state->m_data, sizeof(state->m_data), module->m_meta_data_part_state, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write def part img data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }

        part_transition_count = part->m_transition_count;
        CPE_COPY_HTON16(output_buf + total_sz, &part_transition_count);
        total_sz += 2;

        TAILQ_FOREACH(transition, &part->m_transitions, m_next) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                &transition->m_data, sizeof(transition->m_data), module->m_meta_data_part_transition, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write def part img data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }

    if (ui_string_table_data_size(string_table)) {
        if (total_sz + ui_string_table_data_size(string_table) > output_buf_len) {
            CPE_ERROR(
                em, "save spine-state: write string table not enough output, size=%d, but only %d!",
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
    if (output_buf) mem_free(module->m_alloc, output_buf);
    return rv;
}

static int plugin_spine_data_state_def_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_spine_module_t module = ctx;
    plugin_spine_data_state_def_t def = NULL;
    struct mem_buffer data_buff;
    uint16_t part_count;
    uint16_t i, j;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len <= 0) {
        CPE_ERROR(em, "load def: read file fail!");
        goto COMPLETE;
    }
    input_data = mem_buffer_make_continuous(&data_buff, 0);
    
    def = plugin_spine_data_state_def_create(module, src);
    if (def == NULL) {
        CPE_ERROR(em, "load def: create def fail!");
        goto COMPLETE;
    }

    CPE_COPY_HTON16(&part_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < part_count; ++i) {
        plugin_spine_data_part_t part;
        uint16_t part_state_count;
        uint16_t part_transition_count;

        part = plugin_spine_data_part_create(def);
        if (part == NULL) {
            CPE_ERROR(em, "load def: create part fail!");
            goto COMPLETE;
        }

        if (dr_pbuf_read_with_size(
                &part->m_data, sizeof(part->m_data),
                input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_data_part, em) < 0)
        {
            CPE_ERROR(em, "load def: read part fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        CPE_COPY_HTON16(&part_state_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < part_state_count; ++j) {
            plugin_spine_data_part_state_t part_state;

            part_state = plugin_spine_data_part_state_create(part);
            if (part_state == NULL) {
                CPE_ERROR(em, "load def: create part img fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    &part_state->m_data, sizeof(part_state->m_data),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_data_part_state, em) < 0)
            {
                CPE_ERROR(em, "load def: read part img fail!");
                goto COMPLETE;
            }
            
            total_sz += input_used;
        }

        CPE_COPY_HTON16(&part_transition_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < part_transition_count; ++j) {
            plugin_spine_data_part_transition_t part_transition;

            part_transition = plugin_spine_data_part_transition_create(part);
            if (part_transition == NULL) {
                CPE_ERROR(em, "load def: create part img fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    &part_transition->m_data, sizeof(part_transition->m_data),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_data_part_transition, em) < 0)
            {
                CPE_ERROR(em, "load def: read part img fail!");
                goto COMPLETE;
            }
            
            total_sz += input_used;
        }
    }

    if (ui_data_src_strings_set(src, input_data + total_sz, data_len - total_sz) != 0) {
        CPE_ERROR(em, "load spine-state: load string table fail!");
        goto COMPLETE;
    }
    
    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    return rv;
}

int plugin_spine_data_state_def_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "spine-state.bin", plugin_spine_data_state_def_do_bin_save, ctx, em);
}

int plugin_spine_data_state_def_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "spine-state.bin", plugin_spine_data_state_def_do_bin_load, ctx, em);
}

int plugin_spine_data_state_def_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "spine-state.bin", em);
}
