#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "plugin_particle_data_i.h"

static int plugin_particle_data_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_particle_module_t module = ctx;
    plugin_particle_data_t particle = ui_data_src_product(src);
    ui_string_table_t string_table = ui_data_src_strings(src);
    uint16_t emitter_count = plugin_particle_data_emitter_count(particle);
    uint16_t emitter_mod_count = 0;
    LPDRMETA emitter_meta = plugin_particle_data_emitter_meta(module);
    LPDRMETA emitter_mod_meta = plugin_particle_data_mod_meta(module);
    LPDRMETA point_meta = plugin_particle_module_meta_point(module);
    struct plugin_particle_data_emitter_it emitter_it;
    plugin_particle_data_emitter_t emitter;
    uint16_t curve_point_count = 0;
    struct cpe_hash_it curve_it;
    plugin_particle_data_curve_t curve;
    int output_buf_len;
    char * output_buf = NULL;
    int total_sz = 0;
    int sz;
    int rv = -1;

    if (string_table == NULL) {
        CPE_ERROR(em, " particle bin save: no strings!");
        goto COMPLETE;
    }
    
    /*计算数据大小 */
    plugin_particle_data_emitters(&emitter_it, particle);
    while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
        emitter_mod_count += plugin_particle_data_emitter_mod_count(emitter);
    }

    cpe_hash_it_init(&curve_it, &particle->m_curves);
    while((curve = cpe_hash_it_next(&curve_it))) {
        curve_point_count += curve->m_point_count;
    }
    
    /*分配内存 */
    output_buf_len = (sizeof(UI_PARTICLE_EMITTER) * emitter_count
                      + sizeof(UI_PARTICLE_MOD) * emitter_mod_count
                      + sizeof(UI_CURVE_POINT) * curve_point_count) * 2
        + ui_string_table_data_size(string_table);
    output_buf = mem_alloc(module->m_alloc, output_buf_len);
    if (output_buf == NULL) {
        CPE_ERROR(em, "alloc tmp buff fail!");
        goto COMPLETE;
    }

    /*写数据 */
    /*    EMITTERS */
    CPE_COPY_HTON16(output_buf + total_sz, &emitter_count);
    total_sz += 2;

    plugin_particle_data_emitters(&emitter_it, particle);
    while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
        UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_data_emitter_data(emitter);
        struct plugin_particle_data_mod_it emitter_mod_it;
        plugin_particle_data_mod_t emitter_mod;
    
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_buf_len - total_sz,
            emitter_data, sizeof(*emitter_data), emitter_meta, NULL);
        if (sz < 0) {
            CPE_ERROR(em, "load particle: write emitter data fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        emitter_mod_count = plugin_particle_data_emitter_mod_count(emitter);
        CPE_COPY_HTON16(output_buf + total_sz, &emitter_mod_count);
        total_sz += 2;

        plugin_particle_data_emitter_mods(&emitter_mod_it, emitter);
        while((emitter_mod = plugin_particle_data_mod_it_next(&emitter_mod_it))) {
            UI_PARTICLE_MOD const * emitter_mod_data = plugin_particle_data_mod_data(emitter_mod);
    
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                emitter_mod_data, sizeof(*emitter_mod_data), emitter_mod_meta, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "write particle emitter img data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }

    /*    EMITTERS */
    CPE_COPY_HTON16(output_buf + total_sz, &curve_point_count);
    total_sz += 2;

    cpe_hash_it_init(&curve_it, &particle->m_curves);
    while((curve = cpe_hash_it_next(&curve_it))) {
        uint16_t i;
        for(i = 0; i < curve->m_point_count; ++i) {
            UI_CURVE_POINT const * point_data = &curve->m_points[i];

            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_buf_len - total_sz,
                point_data, sizeof(*point_data), point_meta, NULL);
            if (sz < 0) {
                CPE_ERROR(em, "load particle: write emitter data fail!");
                goto COMPLETE;
            }
            total_sz += sz;
        }
    }

    if (ui_string_table_data_size(string_table)) {
        if (total_sz + ui_string_table_data_size(string_table) > output_buf_len) {
            CPE_ERROR(
                em, "save particle: write string table not enough output, size=%d, but only %d!",
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

static int plugin_particle_data_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_particle_module_t module = ctx;
    plugin_particle_data_t particle = NULL;
    LPDRMETA emitter_meta = plugin_particle_data_emitter_meta(module);
    LPDRMETA emitter_mod_meta = plugin_particle_data_mod_meta(module);
    LPDRMETA point_meta = plugin_particle_module_meta_point(module);
    struct mem_buffer data_buff;
    uint16_t emitter_count;
    uint16_t i, j;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    int rv = -1;
    uint16_t curve_point_count;
    plugin_particle_data_curve_t curve = NULL;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len <= 0) {
        CPE_ERROR(em, "load particle: read file fail!");
        goto COMPLETE;
    }
    input_data = mem_buffer_make_continuous(&data_buff, 0);
    
    particle = plugin_particle_data_create(module, src);
    if (particle == NULL) {
        CPE_ERROR(em, "load particle: create particle fail!");
        goto COMPLETE;
    }

    CPE_COPY_HTON16(&emitter_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < emitter_count; ++i) {
        plugin_particle_data_emitter_t emitter;
        UI_PARTICLE_EMITTER * emitter_data;
        uint16_t emitter_mod_count;

        emitter = plugin_particle_data_emitter_create(particle);
        if (emitter == NULL) {
            CPE_ERROR(em, "load particle: create emitter fail!");
            goto COMPLETE;
        }

        emitter_data = plugin_particle_data_emitter_data(emitter);
        assert(emitter_data);

        if (dr_pbuf_read_with_size(
                emitter_data, sizeof(*emitter_data),
                input_data + total_sz, data_len - total_sz, &input_used, emitter_meta, em) < 0)
        {
            CPE_ERROR(em, "load particle: read emitter fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        CPE_COPY_HTON16(&emitter_mod_count, input_data + total_sz);
        total_sz += 2;

        for(j = 0; j < emitter_mod_count; ++j) {
            plugin_particle_data_mod_t emitter_mod;
            UI_PARTICLE_MOD * emitter_mod_data;

            emitter_mod = plugin_particle_data_mod_create(emitter);
            if (emitter_mod == NULL) {
                CPE_ERROR(em, "load particle: create emitter img fail!");
                goto COMPLETE;
            }

            emitter_mod_data = plugin_particle_data_mod_data(emitter_mod);
            assert(emitter_mod_data);

            if (dr_pbuf_read_with_size(
                    emitter_mod_data, sizeof(*emitter_mod_data),
                    input_data + total_sz, data_len - total_sz, &input_used, emitter_mod_meta, em) < 0)
            {
                CPE_ERROR(em, "load particle: read emitter img fail!");
                goto COMPLETE;
            }
            total_sz += input_used;
        }
    }

    CPE_COPY_HTON16(&curve_point_count, input_data + total_sz);
    total_sz += 2;

    for(i = 0; i < curve_point_count; ++i) {
        UI_CURVE_POINT point_data;
        UI_CURVE_POINT * p;

        if (dr_pbuf_read_with_size(
                &point_data, sizeof(point_data),
                input_data + total_sz, data_len - total_sz, &input_used, point_meta, em) < 0)
        {
            CPE_ERROR(em, "load particle: read point fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        if (curve == NULL || curve->m_id != point_data.curve_id) {
            curve = plugin_particle_data_curve_find(particle, point_data.curve_id);
            if (curve == NULL) {
                curve = plugin_particle_data_curve_create(particle, point_data.curve_id);
                if (curve == NULL) {
                    CPE_ERROR(em, "load particle: create curve fail!");
                    goto COMPLETE;
                }
            }
        }

        p = plugin_particle_data_curve_point_append(curve);
        if (p == NULL) {
            CPE_ERROR(em, "load particle: curve append point fail!");
            goto COMPLETE;
        }
        *p = point_data;
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

int plugin_particle_data_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "particle.bin", plugin_particle_data_do_bin_save, ctx, em);
}

int plugin_particle_data_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "particle.bin", plugin_particle_data_do_bin_load, ctx, em);
}

int plugin_particle_data_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "particle.bin", em);
}
