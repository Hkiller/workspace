#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_layout.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_bin_loader_i.h"

static void ui_data_layout_do_bin_calc_summary(uint16_t * anim_count, uint16_t * anim_frame_count, uint16_t * addition_count, ui_data_control_t control) {
    struct ui_data_control_anim_it anim_it;
    ui_data_control_anim_t anim;
    struct ui_data_control_it child_it;
    ui_data_control_t child_control;

    (*anim_count) += ui_data_control_anim_count(control);
    (*addition_count) += ui_data_control_addition_count(control);
    
    ui_data_control_anims(&anim_it, control);
    while((anim = ui_data_control_anim_it_next(&anim_it))) {
        (*anim_frame_count) += ui_data_control_anim_frame_count(anim);
    }

    ui_data_control_childs(&child_it, control);
    while((child_control = ui_data_control_it_next(&child_it))) {
        ui_data_layout_do_bin_calc_summary(anim_count, anim_frame_count, addition_count, child_control);
    }
}

static int ui_data_layout_do_bin_build_data(
    char * output_buf, int output_buf_len, uint32_t start_pos, uint8_t level,
    ui_data_control_t control, uint16_t parent_index, uint16_t * control_index,
    LPDRMETA control_meta, LPDRMETA control_anim_meta, LPDRMETA control_anim_frame_meta, LPDRMETA control_addition_meta,
    error_monitor_t em)
{
    UI_CONTROL const * control_data = ui_data_control_data(control);
    struct ui_data_control_anim_it anim_it;
    ui_data_control_anim_t anim;
    struct ui_data_control_addition_it addition_it;
    ui_data_control_addition_t addition;
    uint32_t total_sz = 0;
    struct ui_data_control_it child_it;
    ui_data_control_t child_control;
    int sz;
    uint16_t anim_count = ui_data_control_anim_count(control);
    uint16_t addition_count = ui_data_control_addition_count(control);
    uint16_t cur_index = (*control_index)++;
    
    CPE_COPY_HTON16(output_buf + start_pos + total_sz, &parent_index);
    total_sz += 2;
    
    sz = dr_pbuf_write_with_size(
        output_buf + start_pos + total_sz, output_buf_len - start_pos - total_sz,
        control_data, sizeof(*control_data), control_meta, em);
    if (sz < 0) {
        CPE_ERROR(em, "load module: write control data fail!");
        return -1;
    }
    total_sz += sz;

    CPE_COPY_HTON16(output_buf + start_pos + total_sz, &anim_count);
    total_sz += 2;

    ui_data_control_anims(&anim_it, control);
    while((anim = ui_data_control_anim_it_next(&anim_it))) {
        UI_CONTROL_ANIM const * anim_data = ui_data_control_anim_data(anim);
        struct ui_data_control_anim_frame_it anim_frame_it;
        ui_data_control_anim_frame_t anim_frame;
        uint16_t anim_frame_count = ui_data_control_anim_frame_count(anim);
        
        sz = dr_pbuf_write_with_size(
            output_buf + start_pos + total_sz, output_buf_len - start_pos - total_sz,
            anim_data, sizeof(*anim_data), control_anim_meta, em);
        if (sz < 0) {
            CPE_ERROR(em, "load module: write control anim data fail!");
            return -1;
        }
        total_sz += sz;

        CPE_COPY_HTON16(output_buf + start_pos + total_sz, &anim_frame_count);
        total_sz += 2;

        ui_data_control_anim_frames(&anim_frame_it, anim);
        while((anim_frame = ui_data_control_anim_frame_it_next(&anim_frame_it))) {
            UI_CONTROL_ANIM_FRAME const * frame_data = ui_data_control_anim_frame_data(anim_frame);
            
            sz = dr_pbuf_write_with_size(
                output_buf + start_pos + total_sz, output_buf_len - start_pos - total_sz,
                frame_data, sizeof(*frame_data), control_anim_frame_meta, em);
            if (sz < 0) {
                CPE_ERROR(em, "load module: write control anim frame data fail!");
                return -1;
            }
            total_sz += sz;
        }
    }

    CPE_COPY_HTON16(output_buf + start_pos + total_sz, &addition_count);
    total_sz += 2;

    ui_data_control_additions(&addition_it, control);
    while((addition = ui_data_control_addition_it_next(&addition_it))) {
        UI_CONTROL_ADDITION const * addition_data = ui_data_control_addition_data(addition);
        sz = dr_pbuf_write_with_size(
            output_buf + start_pos + total_sz, output_buf_len - start_pos - total_sz,
            addition_data, sizeof(*addition_data), control_addition_meta, em);
        if (sz < 0) {
            CPE_ERROR(em, "load module: write control addition data fail!");
            return -1;
        }
        total_sz += sz;
    }
    
    /* do { */
    /*     char prefix[128]; */
    /*     memset(prefix, ' ', level * 4); */
    /*     prefix[level * 4] = 0; */
    /*     printf("    %d%s%s: used size %d\n", start_pos, prefix, control_data->name, total_sz); */
    /* } while(0); */
    
    ui_data_control_childs(&child_it, control);
    while((child_control = ui_data_control_it_next(&child_it))) {
        sz = ui_data_layout_do_bin_build_data(
            output_buf , output_buf_len, start_pos + total_sz, level + 1,
            child_control, cur_index, control_index,
            control_meta, control_anim_meta, control_anim_frame_meta, control_addition_meta, em);
        if (sz < 0) return -1;

        total_sz += sz;
    }

    return total_sz;
}

static int ui_data_layout_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_bin_loader_t loader = ctx;
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_layout_t layout = ui_data_src_product(src);
    ui_string_table_t string_table = ui_data_layout_strings(layout);
    uint16_t control_count = ui_data_layout_control_count(layout);
    LPDRMETA control_meta = ui_data_control_meta(data_mgr);
    LPDRMETA control_anim_meta = ui_data_control_anim_meta(data_mgr);
    LPDRMETA control_anim_frame_meta = ui_data_control_anim_frame_meta(data_mgr);
    LPDRMETA control_addition_meta = ui_data_control_addition_meta(data_mgr);
    uint16_t control_index = 0;
    uint32_t total_sz;
    int32_t read_sz;
    uint32_t control_sz;
    int rv = -1;
    uint16_t anim_count = 0;
    uint16_t anim_frame_count = 0;
    uint16_t addition_count = 0;
    int output_buf_len;
    char * output_buf = NULL;
    
    ui_data_layout_do_bin_calc_summary(&anim_count, &anim_frame_count, &addition_count, ui_data_layout_root(layout));

    /*分配内存 */
    output_buf_len =
        (sizeof(UI_CONTROL) * control_count
         + sizeof(UI_CONTROL_ANIM) * anim_count
         + sizeof(UI_CONTROL_ANIM_FRAME) * anim_frame_count) * 2
        + ui_string_table_data_size(string_table);
    output_buf = mem_alloc(loader->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    total_sz = 4; /*reserve for sz*/

    read_sz = ui_data_layout_do_bin_build_data(
        output_buf, output_buf_len, total_sz, 0,
        ui_data_layout_root(layout), (uint16_t)-1, &control_index,
        control_meta, control_anim_meta, control_anim_frame_meta, control_addition_meta, em);
    if (read_sz < 0) {
        CPE_ERROR(em, "build control data fail!");
        goto COMPLETE;
    }
    assert(control_index == control_count);
    total_sz += (uint32_t)read_sz;
    control_sz = (uint32_t)read_sz;
    CPE_COPY_HTON32(output_buf, &control_sz);

    if (ui_string_table_data_size(string_table)) {
        if (total_sz + ui_string_table_data_size(string_table) > output_buf_len) {
            CPE_ERROR(
                em, "write string table not enough output, size=%d, but only %d!",
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

static int ui_data_layout_do_bin_load_control(
    ui_data_control_t control, const char * input_data, uint32_t input_size,
    LPDRMETA control_meta, LPDRMETA control_anim_meta, LPDRMETA control_anim_frame_meta, LPDRMETA control_addition_meta, error_monitor_t em)
{
    UI_CONTROL * control_data = ui_data_control_data(control);
    uint32_t used_sz = 0;
    uint16_t anim_count;
    uint16_t anim_pos;
    uint16_t addition_count;
    uint16_t addition_pos;
    size_t sz;
    
    if (dr_pbuf_read_with_size(
            control_data, sizeof(*control_data),
            input_data + used_sz, input_size - used_sz, &sz, control_meta, em)
        < 0)
    {
        CPE_ERROR(em, "load layout: read control data fail!");
        return -1;
    }
    used_sz += sz;

	/* do { */
    /* struct mem_buffer buffer; */
    /* mem_buffer_init(&buffer, NULL); */
    /* printf("xxxxx: control %s\n", dr_json_dump_inline(&buffer, control_data, sizeof(*control_data), control_meta)); */
    /* mem_buffer_clear(&buffer); */
	/* } while(0); */

    CPE_COPY_NTOH16(&anim_count, input_data + used_sz);
    used_sz += 2;

    if (control_data->id != (uint32_t)-1) {
        uint32_t id = control_data->id;
        control_data->id = (uint32_t)-1;
        if (ui_data_control_set_id(control, id) != 0) {
            CPE_ERROR(em, "load layout: control id %d duplicate!", id);
            return -1;
        }
    }
    
    for(anim_pos = 0; anim_pos < anim_count; ++anim_pos) {
        ui_data_control_anim_t anim;
        UI_CONTROL_ANIM * anim_data;
        uint16_t anim_frame_count;
        uint16_t anim_frame_pos;

        anim = ui_data_control_anim_create(control);
        if (anim == NULL) {
            CPE_ERROR(em, "load layout: create control anim fail!");
            return -1;
        }
        anim_data = ui_data_control_anim_data(anim);
        
        if (dr_pbuf_read_with_size(
                anim_data, sizeof(*anim_data),
                input_data + used_sz, input_size - used_sz, &sz, control_anim_meta, em)
            < 0)
        {
            CPE_ERROR(em, "load layout: read control anim data fail!");
            return -1;
        }
        used_sz += sz;

        CPE_COPY_NTOH16(&anim_frame_count, input_data + used_sz);
        used_sz += 2;

        for(anim_frame_pos = 0; anim_frame_pos < anim_frame_count; ++anim_frame_pos) {
            ui_data_control_anim_frame_t anim_frame;
            UI_CONTROL_ANIM_FRAME * frame_data;

            anim_frame = ui_data_control_anim_frame_create(anim);
            if (anim_frame == NULL) {
                CPE_ERROR(em, "load layout: create control anim frame fail!");
                return -1;
            }
            frame_data = ui_data_control_anim_frame_data(anim_frame);
            
            if (dr_pbuf_read_with_size(
                    frame_data, sizeof(*frame_data),
                    input_data + used_sz, input_size - used_sz, &sz, control_anim_frame_meta, em)
                < 0)
            {
                CPE_ERROR(em, "load layout: read control anim frame data fail!");
                return -1;
            }
            used_sz += sz;
        }
    }

    CPE_COPY_NTOH16(&addition_count, input_data + used_sz);
    used_sz += 2;
    for(addition_pos = 0; addition_pos < addition_count; ++addition_pos) {
        ui_data_control_addition_t addition;
        UI_CONTROL_ADDITION * addition_data;

        addition = ui_data_control_addition_create(control);
        if (addition == NULL) {
            CPE_ERROR(em, "load layout: create control addition fail!");
            return -1;
        }
        addition_data = ui_data_control_addition_data(addition);
        
        if (dr_pbuf_read_with_size(
                addition_data, sizeof(*addition_data),
                input_data + used_sz, input_size - used_sz, &sz, control_addition_meta, em)
            < 0)
        {
            CPE_ERROR(em, "load layout: read control addition data fail!");
            return -1;
        }
        used_sz += sz;
    }

    return used_sz;
}

static int ui_data_layout_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    ui_bin_loader_t loader = ctx;
    ui_data_mgr_t data_mgr = ui_data_src_mgr(src);
    ui_data_layout_t layout = NULL;
    struct mem_buffer data_buff;
    LPDRMETA control_meta = ui_data_control_meta(data_mgr);
    LPDRMETA control_anim_meta = ui_data_control_anim_meta(data_mgr);
    LPDRMETA control_anim_frame_meta = ui_data_control_anim_frame_meta(data_mgr);
    LPDRMETA control_addition_meta = ui_data_control_addition_meta(data_mgr);
    ui_data_control_t * control_buf = NULL;
    uint16_t control_buf_capacity = 0;
    uint16_t control_buf_count = 0;
    ssize_t input_len;
    ssize_t used_len;
    uint32_t control_sz;
    const char * input_data;
    int rv = -1;
    
    mem_buffer_init(&data_buff, NULL);

    input_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (input_len < 0) {
        CPE_ERROR(em, "load layout: read file fail!");
        goto COMPLETE;
    }
    input_data = mem_buffer_make_continuous(&data_buff, 0);

    layout = ui_data_layout_create(data_mgr, src);
    if (layout == NULL) {
        CPE_ERROR(em, "load layout: create layout fail!");
        goto COMPLETE;
    }
    
    CPE_COPY_NTOH32(&control_sz, input_data);
    used_len = 4;
    control_sz += 4;
    
    if (control_sz > input_len) {
        CPE_ERROR(em, "load layout: input-len=%d, control-data-len=%d, overflow!", (int)input_len, (int)control_sz);
        goto COMPLETE;
    }
    
    while(used_len < control_sz) {
        uint16_t parent_index;
        int control_used_size;
        ui_data_control_t parent_control;
        ui_data_control_t control;
        
        CPE_COPY_NTOH16(&parent_index, input_data + used_len);
        used_len += 2;

        if (parent_index != (uint16_t)-1) {
            if (parent_index >= control_buf_count) {
                CPE_ERROR(em, "load layout: control parent %d overflow, loaded-count=%d!", parent_index, control_buf_count);
                goto COMPLETE;
            }
            else {
                parent_control = control_buf[parent_index];
            }
        }
        else {
            parent_control = NULL;
        }

        control = ui_data_control_create(layout, parent_control);

        control_used_size =
            ui_data_layout_do_bin_load_control(
                control, input_data + used_len, control_sz - used_len,
                control_meta, control_anim_meta, control_anim_frame_meta, control_addition_meta, em);
        if (control_used_size < 0) {
            CPE_ERROR(em, "load layout: control load fail!");
            goto COMPLETE;
        }

        assert(used_len + control_used_size <= control_sz);
        
        used_len += control_used_size;
        
        if (control_buf_count + 1 > control_buf_capacity) {
            uint16_t new_capacity = control_buf_capacity < 128 ? 128 : control_buf_capacity * 2;
            ui_data_control_t * new_buff = mem_alloc(loader->m_alloc, sizeof(ui_data_control_t) * new_capacity);

            if (control_buf_count) {
                memcpy(new_buff, control_buf, sizeof(ui_data_control_t) * control_buf_count);
                mem_free(loader->m_alloc, control_buf);
            }

            control_buf = new_buff;
            control_buf_capacity = new_capacity;
        }

        control_buf[control_buf_count++] = control;

        /* do { */
        /*     uint16_t level = 0; */
        /*     ui_data_control_t p = ui_data_control_parent(control); */
        /*     while(p) { */
        /*         level++; */
        /*         p = ui_data_control_parent(p); */
        /*     } */

        /*     char prefix[128]; */
        /*     memset(prefix, ' ', level * 4); */
        /*     prefix[level * 4] = 0; */
        /*     printf("    %d%s%s: used size %d\n", used_len - control_used_size - 2, prefix, ui_data_control_name(control), control_used_size + 2); */
        /* } while(0); */
    }

    if (input_len > control_sz) {
        if (ui_string_table_load(ui_data_layout_strings(layout), input_data + control_sz, input_len - control_sz) != 0) {
            CPE_ERROR(em, "load layout: string table load fail!");
            goto COMPLETE;
        }
    }
    
    rv = 0;

COMPLETE:
    if (rv != 0) {
        if (layout) {
            ui_data_layout_free(layout);
        }
    }
    
    if (control_buf) mem_free(loader->m_alloc, control_buf);
    mem_buffer_clear(&data_buff);

    return rv;
}

int ui_data_bin_save_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "lay.bin", ui_data_layout_do_bin_save, ctx, em);
}

int ui_data_bin_load_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "lay.bin", ui_data_layout_do_bin_load, ctx, em);
}

int ui_data_bin_rm_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "lay.bin", em);
}
