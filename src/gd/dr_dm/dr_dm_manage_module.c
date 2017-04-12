#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/utils/id_generator.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_dm/dr_dm_manage.h"
#include "dr_dm_internal_ops.h"

static int dr_dm_manage_app_load_meta(gd_app_context_t app, dr_dm_manage_t mgr, cfg_t cfg) {
    const char * lib_name;
    const char * meta_name;
    const char * id_attr_name;
    dr_ref_t metalib;
    LPDRMETA meta;

    dr_store_manage_t store_mgr;

    store_mgr = dr_store_manage_default(app);
    if (store_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: dr_store_manage not exsit!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    lib_name = cfg_get_string(cfg, "lib-name", NULL);
    if (lib_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: lib-name not configured!",
            dr_dm_manage_name(mgr));
        return -1;
    }
    
    meta_name = cfg_get_string(cfg, "data-meta-name", NULL);
    if (meta_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: data-meta-name not configured!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    id_attr_name = cfg_get_string(cfg, "data-id-attr", NULL);
    if (meta_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: data-id-attr not configured!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    metalib = dr_ref_create(store_mgr, lib_name);
    if (metalib == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: create meta-ref %s fail!",
            dr_dm_manage_name(mgr), lib_name);
        return -1;
    }

    if (dr_ref_lib(metalib) == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: create meta-ref %s fail!",
            dr_dm_manage_name(mgr), lib_name);
        dr_ref_free(metalib);
        return -1;
    }

    meta = dr_lib_find_meta_by_name(dr_ref_lib(metalib), meta_name);
    if (meta == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: metalib %s have no meta %s!",
            dr_dm_manage_name(mgr), lib_name, meta_name);
        dr_ref_free(metalib);
        return -1;
    }
 
    if (dr_dm_manage_set_meta(mgr, meta, metalib) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: set meta to mgr fail!",
            dr_dm_manage_name(mgr));
        dr_ref_free(metalib);
        return -1;
    }
    metalib = NULL;

    if (dr_dm_manage_set_id_attr(mgr, id_attr_name) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: meta %s have no entry %s!",
            dr_dm_manage_name(mgr), meta_name, id_attr_name);
        return -1;
    }

    return 0;
}

static int dr_dm_manage_load_indexes(gd_app_context_t app, dr_dm_manage_t mgr, cfg_t cfg) {
    struct cfg_it it;
    cfg_t child;

    cfg_it_init(&it, cfg);

    while((child = cfg_it_next(&it))) {
        if (cfg_type(child) == CPE_CFG_TYPE_STRING) {
            const char * index_name = cfg_as_string(child, NULL);
            if (index_name == NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load indexes: read index name fail!",
                    dr_dm_manage_name(mgr));
                return -1;
            }

            if (dr_dm_manage_create_index(mgr, index_name, 1) != 0) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load indexes: create index %s fail!",
                    dr_dm_manage_name(mgr), index_name);
                return -1;
            }
        }
        else if (cfg_type(child) == CPE_CFG_TYPE_STRUCT) {
            if (cfg_child_count(child) != 1) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load indexes: struct child count error, count=%d!",
                    dr_dm_manage_name(mgr), cfg_struct_count(child));
                return -1;
            }

            child = cfg_child_only(child);
            assert(child);

            if (dr_dm_manage_create_index(mgr, cfg_name(child), cfg_get_int32(child, "unique", 1)) != 0) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load indexes: create index %s fail!",
                    dr_dm_manage_name(mgr), cfg_name(child));
                return -1;
            }
        }
        else {
            CPE_ERROR(
                gd_app_em(app), "%s: load indexes: not support cfg type %d!",
                dr_dm_manage_name(mgr), cfg_type(child));
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
int dr_dm_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    const char * id_generate_name;
    cfg_t meta_cfg;
    dr_dm_manage_t dr_dm_manage;

    dr_dm_manage = dr_dm_manage_create(
        app,
        gd_app_module_name(module),
        gd_app_alloc(app),
        gd_app_em(app));
    if (dr_dm_manage == NULL) return -1;

    if ((meta_cfg = cfg_find_cfg(cfg, "meta"))) {
        if (dr_dm_manage_app_load_meta(app, dr_dm_manage, meta_cfg) != 0) {
            dr_dm_manage_free(dr_dm_manage);
            return -1;
        }
    }

    if (dr_dm_manage_load_indexes(app, dr_dm_manage, cfg_find_cfg(cfg, "indexes")) != 0) {
        return -1;
    }

    if ((id_generate_name = cfg_get_string(cfg, "id-generate", NULL))) {
        gd_id_generator_t id_generator = gd_id_generator_find_nc(app, id_generate_name);
        if (id_generator == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: id-generate %s not exist!",
                dr_dm_manage_name(dr_dm_manage), id_generate_name);
            return -1;
        }

        dr_dm_manage_set_id_generate(dr_dm_manage, id_generator);
    }
    
    dr_dm_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (dr_dm_manage->m_debug) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, 0);

        if (cpe_hash_table_count(&dr_dm_manage->m_indexes)) {
            struct write_stream_buffer stream;
            struct cpe_hash_it index_it;
            struct dr_dm_data_index * index;

            write_stream_buffer_init(&stream, &buffer);

            cpe_hash_it_init(&index_it, &dr_dm_manage->m_indexes);
            index = cpe_hash_it_next(&index_it);
            stream_printf((write_stream_t)&stream, "%s", index->m_name);

            while((index = cpe_hash_it_next(&index_it))) {
                stream_printf((write_stream_t)&stream, ", %s", index->m_name);
            }

            stream_putc((write_stream_t)&stream, 0);
        }
        
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. meta=%s, key=%s, id-generate=%s, indexes=[%s]",
            gd_app_module_name(module),
            dr_dm_manage_meta(dr_dm_manage) ? dr_meta_name(dr_dm_manage_meta(dr_dm_manage)) : "???",
            dr_dm_manage_id_attr(dr_dm_manage) ? dr_entry_name(dr_dm_manage_id_attr(dr_dm_manage)) : "???",
            dr_dm_manage_id_generate(dr_dm_manage) ? gd_id_generator_name(dr_dm_manage_id_generate(dr_dm_manage)) : "???",
            mem_buffer_size(&buffer) ? (const char *)mem_buffer_make_continuous(&buffer, 0) : "");

        mem_buffer_clear(&buffer);
    }

    return 0;
}

EXPORT_DIRECTIVE
void dr_dm_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dr_dm_manage_t dr_dm_manage;

    dr_dm_manage = dr_dm_manage_find_nc(app, gd_app_module_name(module));
    if (dr_dm_manage) {
        dr_dm_manage_free(dr_dm_manage);
    }
}
