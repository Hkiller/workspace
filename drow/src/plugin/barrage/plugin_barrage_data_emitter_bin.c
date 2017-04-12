#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/barrage/plugin_barrage_module.h"
#include "plugin_barrage_data_barrage_i.h"
#include "plugin_barrage_data_emitter_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static int plugin_barrage_data_barrage_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_barrage_module_t module = (plugin_barrage_module_t)ctx;
    plugin_barrage_data_barrage_t barrage = (plugin_barrage_data_barrage_t)ui_data_src_product(src);
    char * output_buf = NULL;
    plugin_barrage_data_emitter_t emitter;
    uint16_t emitter_count = 0;
    uint16_t emitter_trigger_count = 0;
    uint16_t bullet_trigger_count = 0;
    int rv = -1;
    int sz;
    size_t output_sz;
    int total_sz;

    /*统计数据 */
    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next) {
        emitter_count++;
        emitter_trigger_count += emitter->m_emitter_trigger_count;
        bullet_trigger_count += emitter->m_bullet_trigger_count;
    }

    /*分配输出缓冲 */
    output_sz =
        ( sizeof(BARRAGE_BARRAGE_INFO)
          + sizeof(BARRAGE_EMITTER_INFO) * emitter_count
          + sizeof(BARRAGE_EMITTER_EMITTER_TRIGGER_INFO) * emitter_trigger_count
          + sizeof(BARRAGE_EMITTER_BULLET_TRIGGER_INFO) * bullet_trigger_count)
        * 2;
    output_buf = (char*)mem_alloc(module->m_alloc, output_sz);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc fail!");
        goto COMPLETE;
    }

    total_sz = 0;

    /*head */
    sz = dr_pbuf_write_with_size(
        output_buf, output_sz,
        plugin_barrage_data_barrage_data(barrage),
        sizeof(*plugin_barrage_data_barrage_data(barrage)),
        module->m_meta_barrage_info, em);
    if (sz < 0) {
        CPE_ERROR(em, "pbuf write fail!");
        goto COMPLETE;
    }
    total_sz += sz;

    /*bullet emitters*/
    CPE_COPY_HTON16(output_buf + total_sz, &emitter_count);
    total_sz += 2;
    
    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next) {
        BARRAGE_EMITTER_INFO * emitter_data;
        struct plugin_barrage_data_emitter_trigger_it emitter_trigger_it;
        plugin_barrage_data_emitter_trigger_t emitter_trigger;
        struct plugin_barrage_data_bullet_trigger_it bullet_trigger_it;
        plugin_barrage_data_bullet_trigger_t bullet_trigger;
        
        emitter_data = plugin_barrage_data_emitter_data(emitter);
        assert(emitter_data);

        /*head */
        sz = dr_pbuf_write_with_size(output_buf + total_sz, output_sz - total_sz, emitter_data, sizeof(*emitter_data), module->m_meta_emitter_info, em);
        if (sz < 0) {
            CPE_ERROR(em, "pbuf write fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        /*emitter triggers*/
        CPE_COPY_HTON16(output_buf + total_sz, &emitter->m_emitter_trigger_count);
        total_sz += 2;

        plugin_barrage_data_emitter_triggers(&emitter_trigger_it, emitter);
        while((emitter_trigger = plugin_barrage_data_emitter_trigger_it_next(&emitter_trigger_it))) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_sz - total_sz,
                plugin_barrage_data_emitter_trigger_data(emitter_trigger),
                sizeof(*plugin_barrage_data_emitter_trigger_data(emitter_trigger)),
                module->m_meta_emitter_trigger_info,
                em);
            if (sz < 0) {
                CPE_ERROR(em, "pbuf write fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }

        /*bullet triggers*/
        CPE_COPY_HTON16(output_buf + total_sz, &emitter->m_bullet_trigger_count);
        total_sz += 2;

        plugin_barrage_data_bullet_triggers(&bullet_trigger_it, emitter);
        while((bullet_trigger = plugin_barrage_data_bullet_trigger_it_next(&bullet_trigger_it))) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_sz - total_sz,
                plugin_barrage_data_bullet_trigger_data(bullet_trigger),
                sizeof(*plugin_barrage_data_bullet_trigger_data(bullet_trigger)),
                module->m_meta_bullet_trigger_info,
                em);
            if (sz < 0) {
                CPE_ERROR(em, "pbuf write bullet trigger fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }

    if (vfs_file_write(fp, output_buf, total_sz) != total_sz) {
        CPE_ERROR(em, "write to file fail!");
        goto COMPLETE;
    }
    
    rv = 0;

COMPLETE:
    if (output_buf) mem_free(module->m_alloc, output_buf);

    return rv;
}

static int plugin_barrage_data_barrage_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_barrage_module_t module = (plugin_barrage_module_t)ctx;
    plugin_barrage_data_barrage_t barrage = NULL;
    struct mem_buffer data_buff;
    ssize_t data_len;
    char * input_data;
    uint16_t emitter_count;
    uint16_t i, j;
    int rv = -1;
    size_t input_used;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len < 0) {
        CPE_ERROR(em, "load barrage: read file fail!");
        goto COMPLETE;
    } 
    input_data = (char*)mem_buffer_make_continuous(&data_buff, 0);
    
    barrage = plugin_barrage_data_barrage_create(module, src);
    if (barrage == NULL) {
        CPE_ERROR(em, "load barrage: create emitter fail!");
        goto COMPLETE;
    }

    /*barrage */
    if (dr_pbuf_read_with_size(
            plugin_barrage_data_barrage_data(barrage),
            sizeof(*plugin_barrage_data_barrage_data(barrage)),
            input_data, data_len, &input_used, module->m_meta_barrage_info, em) < 0)
    {
        CPE_ERROR(em, "load barrage: read record fail!");
        goto COMPLETE;
    }
    total_sz += input_used;

    CPE_COPY_HTON16(&emitter_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < emitter_count; ++i) {
        plugin_barrage_data_emitter_t emitter;
        uint16_t emitter_trigger_count;
        uint16_t bullet_trigger_count;
        BARRAGE_EMITTER_INFO * emitter_data;
        
        emitter = plugin_barrage_data_emitter_create(barrage);
        if (emitter == NULL) {
            CPE_ERROR(em, "load barrage: create emitter fail!");
            goto COMPLETE;
        }
        emitter_data = plugin_barrage_data_emitter_data(emitter);
        assert(emitter_data);
        
        /*emitter */
        if (dr_pbuf_read_with_size(
                emitter_data, sizeof(*emitter_data),
                input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_emitter_info, em) < 0)
        {
            CPE_ERROR(em, "load barrage: read record fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        /*emitter trigger */
        CPE_COPY_HTON16(&emitter_trigger_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < emitter_trigger_count; ++j) {
            plugin_barrage_data_emitter_trigger_t trigger;

            trigger = plugin_barrage_data_emitter_trigger_create(emitter);
            if (trigger == NULL) {
                CPE_ERROR(em, "load barrage: create emitter trigger fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    plugin_barrage_data_emitter_trigger_data(trigger), sizeof(*plugin_barrage_data_emitter_trigger_data(trigger)),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_emitter_trigger_info, em) < 0)
            {
                CPE_ERROR(em, "load barrage: read record fail!");
                goto COMPLETE;
            }
            total_sz += input_used;

            if (plugin_barrage_data_emitter_trigger_update(trigger) != 0) {
                CPE_ERROR(em, "load barrage: update trigger fail!");
                goto COMPLETE;
            }
        }

        /*bullet bullet */
        CPE_COPY_HTON16(&bullet_trigger_count, input_data + total_sz);
        total_sz += 2;
        for(j = 0; j < bullet_trigger_count; ++j) {
            plugin_barrage_data_bullet_trigger_t trigger;

            trigger = plugin_barrage_data_bullet_trigger_create(emitter);
            if (trigger == NULL) {
                CPE_ERROR(em, "load barrage: create bullet trigger fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    plugin_barrage_data_bullet_trigger_data(trigger), sizeof(*plugin_barrage_data_bullet_trigger_data(trigger)),
                    input_data + total_sz, data_len - total_sz, &input_used, module->m_meta_bullet_trigger_info, em) < 0)
            {
                CPE_ERROR(em, "load barrage: read record fail!");
                goto COMPLETE;
            }
            total_sz += input_used;

            if (plugin_barrage_data_bullet_trigger_update(trigger) != 0) {
                CPE_ERROR(em, "load barrage: update trigger fail!");
                goto COMPLETE;
            }
        }

        if ((emitter_data->frame_duration.base + emitter_data->frame_duration.adj) <= 0) {
            CPE_ERROR(
                em, "load barrage: data error: %s",
                dr_json_dump_inline(&module->m_dump_buffer, emitter_data, sizeof(*emitter_data), module->m_meta_emitter_info));
            goto COMPLETE;
        }
    }
    
    /* if (plugin_barrage_data_barrage_update_using(barrage) != 0) { */
    /*     CPE_ERROR(em, "load barrage: update emitter refs fail"); */
    /*     goto COMPLETE; */
    /* } */
    
    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    if (rv != 0 && barrage) {
        plugin_barrage_data_barrage_free(barrage);
    }

    return rv;
}

int plugin_barrage_data_barrage_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "barrage.bin", plugin_barrage_data_barrage_do_bin_save, ctx, em);
}

int plugin_barrage_data_barrage_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "barrage.bin", plugin_barrage_data_barrage_do_bin_load, ctx, em);
}

#ifdef __cplusplus
}
#endif
