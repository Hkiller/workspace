#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_pbuf.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin_chipmunk_data_scene_i.h"
#include "plugin_chipmunk_data_body_i.h"
#include "plugin_chipmunk_data_fixture_i.h"
#include "plugin_chipmunk_data_polygon_node_i.h"
#include "plugin_chipmunk_data_constraint_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static int plugin_chipmunk_data_scene_do_bin_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_chipmunk_module_t module = (plugin_chipmunk_module_t)ctx;
    plugin_chipmunk_data_scene_t scene = (plugin_chipmunk_data_scene_t)ui_data_src_product(src);
    plugin_chipmunk_data_body_t body;
    plugin_chipmunk_data_fixture_t fixture;
    plugin_chipmunk_data_polygon_node_t polygon_node;
    plugin_chipmunk_data_constraint_t constraint;
    LPDRMETA scene_meta = plugin_chipmunk_module_data_scene_meta(module);
    LPDRMETA body_meta = plugin_chipmunk_module_data_body_meta(module);
    LPDRMETA fixture_meta = plugin_chipmunk_module_data_fixture_meta(module);
    LPDRMETA polygon_node_meta = plugin_chipmunk_module_data_polygon_node_meta(module);
    LPDRMETA constraint_meta = plugin_chipmunk_module_data_constraint_meta(module);
    char * output_buf = NULL;
    size_t output_sz;
    int sz;
    int total_sz;
    int rv = -1;

    /*alloc buf*/
    output_sz = sizeof(scene->m_data) + sizeof(CHIPMUNK_BODY) * scene->m_body_count;
    TAILQ_FOREACH(body, &scene->m_body_list, m_next_for_scene) {
        output_sz += sizeof(CHIPMUNK_FIXTURE) * body->m_fixture_count;
        TAILQ_FOREACH(fixture, &body->m_fixture_list, m_next_for_body) {
            output_sz += sizeof(CHIPMUNK_POLYGON_NODE) * fixture->m_polygon_node_count;
        }
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

    /*body*/
    CPE_COPY_HTON16(output_buf + total_sz, &scene->m_body_count);
    total_sz += 2;
    
    TAILQ_FOREACH(body, &scene->m_body_list, m_next_for_scene) {
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_sz - total_sz,
            &body->m_data, sizeof(body->m_data), body_meta, em);
        if (sz < 0) {
            CPE_ERROR(em, "pbuf write body fail!");
            goto COMPLETE;
        }
        total_sz += sz;

        CPE_COPY_HTON16(output_buf + total_sz, &body->m_fixture_count);
        total_sz += 2;

        /*fixture*/        
        TAILQ_FOREACH(fixture, &body->m_fixture_list, m_next_for_body) {
            sz = dr_pbuf_write_with_size(
                output_buf + total_sz, output_sz - total_sz,
                &fixture->m_data, sizeof(fixture->m_data), fixture_meta, em);
            if (sz < 0) {
                CPE_ERROR(em, "pbuf write fixture fail!");
                goto COMPLETE;
            }
            total_sz += sz;

            /*polygon node*/
            if (fixture->m_data.fixture_type == chipmunk_fixture_type_polygon) {
                
                CPE_COPY_HTON16(output_buf + total_sz, &fixture->m_polygon_node_count);
                total_sz += 2;

                TAILQ_FOREACH(polygon_node, &fixture->m_polygon_node_list, m_next_for_fixture) {
                    sz = dr_pbuf_write_with_size(
                        output_buf + total_sz, output_sz - total_sz,
                        &polygon_node->m_data, sizeof(polygon_node->m_data), polygon_node_meta, em);
                    if (sz < 0) {
                        CPE_ERROR(em, "pbuf write polygon node fail!");
                        goto COMPLETE;
                    }
                    total_sz += sz;
                }
            }
        }
    }

    /*constraint*/
    CPE_COPY_HTON16(output_buf + total_sz, &scene->m_constraint_count);
    total_sz += 2;

    TAILQ_FOREACH(constraint, &scene->m_constraint_list, m_next_for_scene) {
        sz = dr_pbuf_write_with_size(
            output_buf + total_sz, output_sz - total_sz,
            &constraint->m_data, sizeof(constraint->m_data), constraint_meta, em);
        if (sz < 0) {
            CPE_ERROR(em, "pbuf write constraint fail!");
            goto COMPLETE;
        }
        total_sz += sz;
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

static int plugin_chipmunk_data_scene_do_bin_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_chipmunk_module_t module = (plugin_chipmunk_module_t)ctx;
    plugin_chipmunk_data_scene_t scene = NULL;
    plugin_chipmunk_data_body_t body;
    plugin_chipmunk_data_fixture_t fixture;
    plugin_chipmunk_data_polygon_node_t polygon_node;
    plugin_chipmunk_data_constraint_t constraint;
    LPDRMETA scene_meta = plugin_chipmunk_module_data_scene_meta(module);
    LPDRMETA body_meta = plugin_chipmunk_module_data_body_meta(module);
    LPDRMETA fixture_meta = plugin_chipmunk_module_data_fixture_meta(module);
    LPDRMETA polygon_node_meta = plugin_chipmunk_module_data_polygon_node_meta(module);
    LPDRMETA constraint_meta = plugin_chipmunk_module_data_constraint_meta(module);
    struct mem_buffer data_buff;
    ssize_t data_len;
    char * input_data;
    size_t input_used;
    uint16_t body_count;
    uint16_t fixture_count;
    uint16_t polygon_node_count;    
    uint16_t body_i;
    uint16_t fixture_i;
    uint16_t polygon_node_i;    
    uint16_t constraint_count;
    uint16_t constraint_i;
    int rv = -1;
    int total_sz = 0;

    mem_buffer_init(&data_buff, NULL);

    data_len = vfs_file_load_to_buffer(&data_buff, fp);
    if (data_len < 0) {
        CPE_ERROR(em, "load chipmunk: read file fail!");
        goto COMPLETE;
    } 
    input_data = (char*)mem_buffer_make_continuous(&data_buff, 0);

    scene = plugin_chipmunk_data_scene_create(module, src);
    if (scene == NULL) {
        CPE_ERROR(em, "load chipmunk: create scene fail!");
        goto COMPLETE;
    }

    /*scene*/
    if (dr_pbuf_read_with_size(&scene->m_data, sizeof(scene->m_data), input_data, data_len, &input_used, scene_meta, em) < 0) {
        CPE_ERROR(em, "load chipmunk: read scene fail!");
        goto COMPLETE;
    }
    total_sz += input_used;

    /*body*/
    CPE_COPY_HTON16(&body_count, input_data + total_sz);
    total_sz += 2;

    for(body_i = 0; body_i < body_count; ++body_i) {
        body = plugin_chipmunk_data_body_create(scene);
        if (body == NULL) {
            CPE_ERROR(em, "load chipmunk: create body fail!");
            goto COMPLETE;
        }

        if (dr_pbuf_read_with_size(
                &body->m_data, sizeof(body->m_data),
                input_data + total_sz, data_len - total_sz, &input_used, body_meta, em) < 0)
        {
            CPE_ERROR(em, "load chipmunk: read body fail!");
            goto COMPLETE;
        }
        total_sz += input_used;

        /*fixture*/
        CPE_COPY_HTON16(&fixture_count, input_data + total_sz);
        total_sz += 2;

        for(fixture_i = 0; fixture_i < fixture_count; ++fixture_i) {
            fixture = plugin_chipmunk_data_fixture_create(body);
            if (fixture == NULL) {
                CPE_ERROR(em, "load chipmunk: create fixture fail!");
                goto COMPLETE;
            }

            if (dr_pbuf_read_with_size(
                    &fixture->m_data, sizeof(fixture->m_data),
                    input_data + total_sz, data_len - total_sz, &input_used, fixture_meta, em) < 0)
            {
                CPE_ERROR(em, "load chipmunk: read fixture fail!");
                goto COMPLETE;
            }
            total_sz += input_used;

            /*polygon_node*/
            if (fixture->m_data.fixture_type == chipmunk_fixture_type_polygon) {
                CPE_COPY_HTON16(&polygon_node_count, input_data + total_sz);
                total_sz += 2;

                for(polygon_node_i = 0; polygon_node_i < polygon_node_count; ++polygon_node_i) {
                    polygon_node = plugin_chipmunk_data_polygon_node_create(fixture);
                    if (polygon_node == NULL) {
                        CPE_ERROR(em, "load chipmunk: create polygon_node fail!");
                        goto COMPLETE;
                    }

                    if (dr_pbuf_read_with_size(
                            &polygon_node->m_data, sizeof(polygon_node->m_data),
                            input_data + total_sz, data_len - total_sz, &input_used, polygon_node_meta, em) < 0)
                    {
                        CPE_ERROR(em, "load chipmunk: read polygon_node fail!");
                        goto COMPLETE;
                    }
                    total_sz += input_used;
                }
            }
        }
    }

    /*constraint*/
    CPE_COPY_HTON16(&constraint_count, input_data + total_sz);
    total_sz += 2;

    for(constraint_i = 0; constraint_i < constraint_count; ++constraint_i) {
        constraint = plugin_chipmunk_data_constraint_create(scene);
        if (constraint == NULL) {
            CPE_ERROR(em, "load chipmunk: create constraint fail!");
            goto COMPLETE;
        }

        if (dr_pbuf_read_with_size(
                &constraint->m_data, sizeof(constraint->m_data),
                input_data + total_sz, data_len - total_sz, &input_used, constraint_meta, em) < 0)
        {
            CPE_ERROR(em, "load chipmunk: read constraint fail!");
            goto COMPLETE;
        }
        total_sz += input_used;
    }
    
    rv = 0;

COMPLETE:
    mem_buffer_clear(&data_buff);

    return rv;
}

int plugin_chipmunk_data_scene_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "chipmunk.bin", plugin_chipmunk_data_scene_do_bin_save, ctx, em);
}

int plugin_chipmunk_data_scene_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "chipmunk.bin", plugin_chipmunk_data_scene_do_bin_load, ctx, em);
}

int plugin_chipmunk_data_scene_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "chipmunk.bin", em);
}

#ifdef __cplusplus
}
#endif
