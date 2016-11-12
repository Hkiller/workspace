#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "rank_f_svr_ops.h"

static int rank_f_svr_load_def(rank_f_svr_t svr, cfg_t cfg);
static int rank_f_svr_build_record_meta(rank_f_svr_t svr);
static int rank_f_svr_record_init(rank_f_svr_t svr);

EXPORT_DIRECTIVE
int rank_f_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    rank_f_svr_t rank_f_svr;
    const char * request_recv_at;
    cfg_t rank_def;

    if ((request_recv_at = cfg_get_string(cfg, "request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: rank_f-request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    rank_f_svr =
        rank_f_svr_create(
            app, gd_app_module_name(module), stub,
            gd_app_alloc(app), gd_app_em(app));
    if (rank_f_svr == NULL) return -1;

    rank_f_svr->m_debug = cfg_get_int8(cfg, "debug", rank_f_svr->m_debug);

    if (rank_f_svr_set_request_recv_at(rank_f_svr, request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set request-recv-at %s fail!", gd_app_module_name(module), request_recv_at);
        rank_f_svr_free(rank_f_svr);
        return -1;
    }

    rank_def = cfg_find_cfg(gd_app_cfg(app), "rank-def");
    if (rank_def == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: rank-def not configured.", gd_app_module_name(module));
        rank_f_svr_free(rank_f_svr);
        return -1;
    }

    if (rank_f_svr_load_def(rank_f_svr, rank_def) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load rank-def fail.", gd_app_module_name(module));
        rank_f_svr_free(rank_f_svr);
        return -1;
    }

    if (rank_f_svr_build_record_meta(rank_f_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: build record meta fail.", gd_app_module_name(module));
        rank_f_svr_free(rank_f_svr);
        return -1;
    }

    if (rank_f_svr_record_init(rank_f_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: init record fail.", gd_app_module_name(module));
        rank_f_svr_free(rank_f_svr);
        return -1;
    }

    if (rank_f_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void rank_f_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    rank_f_svr_t rank_f_svr;

    rank_f_svr = rank_f_svr_find_nc(app, gd_app_module_name(module));
    if (rank_f_svr) {
        rank_f_svr_free(rank_f_svr);
    }
}

static LPDRMETA rank_f_svr_load_meta(rank_f_svr_t svr, const char * str_meta) {
    dr_store_manage_t store_mgr;
    dr_store_t store;
    char const * sep;
    char lib_name[64];
    LPDRMETA meta;

    store_mgr = dr_store_manage_find_nc(svr->m_app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: store_mgr not exist!", rank_f_svr_name(svr));
        return NULL;
    }

    sep = strchr(str_meta, '.');
    if (sep == NULL || (sep - str_meta) > (sizeof(lib_name) - 1)) {
        CPE_ERROR(svr->m_em, "%s: pkg-meta %s format error or overflow!", rank_f_svr_name(svr), str_meta);
        return NULL;
    }
    memcpy(lib_name, str_meta, sep - str_meta);
    lib_name[sep - str_meta] = 0;

    store = dr_store_find(store_mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s not exist in %s!",
            rank_f_svr_name(svr), lib_name, dr_store_manage_name(store_mgr));
        return NULL;
    }

    meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s have no meta %s!",
            rank_f_svr_name(svr), lib_name, sep + 1);
        return NULL;
    }

    return meta;
}

static int rank_f_svr_load_def(rank_f_svr_t svr, cfg_t cfg) {
    const char * str_meta;
    const char * str_uid_entry;
    struct cfg_it indexes_it;
    cfg_t index_cfg;
    rank_f_svr_index_info_t gid_index_info;

    str_meta = cfg_get_string(cfg, "meta", NULL);
    if (str_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: rank-def.meta not configured.", rank_f_svr_name(svr));
        return -1;
    }

    str_uid_entry = cfg_get_string(cfg, "user-id", NULL);
    if (str_uid_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: rank-def.user-id not configured.", rank_f_svr_name(svr));
        return -1;
    }

    svr->m_data_meta = rank_f_svr_load_meta(svr, str_meta);
    if (svr->m_data_meta == NULL) return -1;
    svr->m_data_size = dr_meta_size(svr->m_data_meta);

    gid_index_info = rank_f_svr_index_info_create(svr, 0);
    assert(gid_index_info);
    if (rank_f_svr_index_info_add_sorter(gid_index_info, str_uid_entry, "asc") != 0) {
        CPE_ERROR(svr->m_em, "%s: create: create gid index info: sorter create fail", rank_f_svr_name(svr));
        return -1;
    }

    cfg_it_init(&indexes_it, cfg_find_cfg(cfg, "indexes"));
    while((index_cfg = cfg_it_next(&indexes_it))) {
        rank_f_svr_index_info_t index_info;
        uint16_t id;
        struct cfg_it sorters_it;
        cfg_t sorter_cfg;

        if (cfg_try_get_uint16(index_cfg, "id", &id) != 0) {
            CPE_ERROR(svr->m_em, "%s: create: create index info: id not configured", rank_f_svr_name(svr));
            return -1;
        }

        index_info = rank_f_svr_index_info_create(svr, id);
        if (index_info == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: create index info %d fail", rank_f_svr_name(svr), id);
            return -1;
        }


        cfg_it_init(&sorters_it, cfg_find_cfg(index_cfg, "sorters"));
        while((sorter_cfg = cfg_it_next(&sorters_it))) {
            const char * attr = cfg_get_string(sorter_cfg, "attr", NULL);
            const char * order = cfg_get_string(sorter_cfg, "order", NULL);

            if (attr == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: create index info %d: sorter.attr not configured", rank_f_svr_name(svr), id);
                return -1;
            }

            if (order == NULL) {
                CPE_ERROR(svr->m_em, "%s: create: create index info %d: sorter.order not configured", rank_f_svr_name(svr), id);
                return -1;
            }

            if (rank_f_svr_index_info_add_sorter(index_info, attr, order) != 0) {
                CPE_ERROR(svr->m_em, "%s: create: create index info %d: sorter(%s, %s) create fail", rank_f_svr_name(svr), id, attr, order);
                return -1;
            }
        }
    }

    return 0;
}

extern char g_metalib_svr_rank_f_pro[];
int rank_f_svr_build_record_meta(rank_f_svr_t svr) {
    struct DRInBuildMetaLib * in_build_lib;
    struct DRInBuildMeta * in_build_meta;
    LPDRMETA src_record_meta;
    LPDRMETALIB metalib;

    src_record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_rank_f_pro, "svr_rank_f_record");
    if (src_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: meta svr_rank_f_record not exist", rank_f_svr_name(svr));
        return -1;
    }

    in_build_lib = dr_inbuild_create_lib();
    if (in_build_lib == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: create builder fail", rank_f_svr_name(svr));
        return -1;
    }

    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: create meta record in metalib fail", rank_f_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "record");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_data_meta));

    char keys_buf[128];
    snprintf(keys_buf, sizeof(keys_buf), "rank_f_uid,%s", dr_entry_name(svr->m_index_infos[0].m_sorters[0].m_sort_entry));
    dr_inbuild_meta_add_key_entries(in_build_meta, keys_buf);

    if (dr_inbuild_meta_copy_entrys(in_build_meta, src_record_meta) != 0
        || dr_inbuild_meta_copy_entrys(in_build_meta, svr->m_data_meta) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: copy entries fail", rank_f_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }
        
    if (dr_inbuild_tsort(in_build_lib, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: sort record metalib fail", rank_f_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    if (dr_inbuild_build_lib(&svr->m_record_metalib, in_build_lib, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: build record metalib fail", rank_f_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    metalib = (LPDRMETALIB)mem_buffer_make_continuous(&svr->m_record_metalib, 0);
    svr->m_record_meta = dr_lib_find_meta_by_name(metalib, "record");
    if (svr->m_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: find record in metalib fail", rank_f_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    if (dr_meta_size(svr->m_record_meta) != dr_meta_size(src_record_meta) + dr_meta_size(svr->m_data_meta)) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, svr->m_alloc);

        CPE_ERROR(
            svr->m_em, "%s: create: build record meta: data size mismatch:"
            " svr_rank_f_record.size=%d, %s.size=%d, record.size=%d\n%s",
            rank_f_svr_name(svr), (int)dr_meta_size(src_record_meta),
            dr_meta_name(svr->m_data_meta), (int)dr_meta_size(svr->m_data_meta),
            (int)dr_meta_size(svr->m_record_meta),
            dr_lib_dump(&buffer, metalib, 4));

        dr_inbuild_free_lib(in_build_lib);

        mem_buffer_clear(&buffer);

        return -1;
    }

    dr_inbuild_free_lib(in_build_lib);

    svr->m_record_size = dr_meta_size(svr->m_record_meta);
    assert(svr->m_record_size == svr->m_data_size + sizeof(SVR_RANK_F_RECORD));

    if (svr->m_debug) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, svr->m_alloc);

        CPE_INFO(svr->m_em, "%s: create: record meta:\n%s", rank_f_svr_name(svr), dr_lib_dump(&buffer, metalib, 4));

        mem_buffer_clear(&buffer);
    }

    return 0;
}

static int rank_f_svr_record_init(rank_f_svr_t svr) {
    const char * str_mem_size;
    uint64_t mem_size;

    str_mem_size = gd_app_arg_find(svr->m_app, "--mem-size");
    if (str_mem_size == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: record init: not support init from shm", rank_f_svr_name(svr));
        return -1;
    }

    if (cpe_str_parse_byte_size(&mem_size, str_mem_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: record init: mem-size %s format error", rank_f_svr_name(svr), str_mem_size);
        return -1;
    }

    if (gd_app_arg_is_set(svr->m_app, "--shm")) {
        CPE_ERROR(svr->m_em, "%s: create: record init: not support init from shm", rank_f_svr_name(svr));
        return -1;
    }
    else {
        if (rank_f_svr_record_init_from_mem(svr, (size_t)mem_size) != 0) {
            CPE_ERROR(svr->m_em, "%s: create: record init: init from mem (size=%d) fail", rank_f_svr_name(svr), (int)mem_size);
            return -1;
        }

        return 0;
    }
}
