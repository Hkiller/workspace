#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/time_utils.h"
#include "cpe/utils/tailq_sort.h"
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
#include "svr/set/logic/set_logic_sp.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "rank_g_svr.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_season_info.h"
#include "rank_g_svr_rank_tree.h"

extern char g_metalib_svr_rank_g_pro[];
static int rank_g_svr_load_def(rank_g_svr_t svr, cfg_t cfg);
static int rank_g_svr_load_seasons(rank_g_svr_t svr, rank_g_svr_index_t index, cfg_t cfg);
static int rank_g_svr_load_record(rank_g_svr_t svr);

EXPORT_DIRECTIVE
int rank_g_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    rank_g_svr_t rank_g_svr;
    set_logic_sp_t set_sp;
    set_logic_rsp_manage_t rsp_manage;
    mongo_cli_proxy_t db;
    cfg_t rank_def;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    set_sp = set_logic_sp_find_nc(app, cfg_get_string(cfg, "set-sp", NULL));
    if (set_sp == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-sp %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-sp", "default"));
        return -1;
    }

    rsp_manage = set_logic_rsp_manage_find_nc(app, cfg_get_string(cfg, "rsp-manage", NULL));
    if (rsp_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: rsp-manage %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "rsp-manage", "default"));
        return -1;
    }

    db = mongo_cli_proxy_find_nc(app, cfg_get_string(cfg, "db", NULL));
    if (db == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: db %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "db", "default"));
        return -1;
    }
    
    rank_g_svr =
        rank_g_svr_create(
            app, gd_app_module_name(module), stub, set_sp, rsp_manage, db,
            gd_app_alloc(app), gd_app_em(app));
    if (rank_g_svr == NULL) return -1;

    if (set_logic_rsp_build(
            rank_g_svr->m_rsp_manage,
            cfg_find_cfg(gd_app_cfg(app), "rsps"), (LPDRMETALIB)g_metalib_svr_rank_g_pro, gd_app_em(app)) != 0)
    {
        CPE_ERROR(gd_app_em(app), "%s: create: load rsps fail!", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }
    
    rank_g_svr->m_debug = cfg_get_int8(cfg, "debug", rank_g_svr->m_debug);

    rank_def = cfg_find_cfg(gd_app_cfg(app), "rank-def");
    if (rank_def == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: rank-def not configured.", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    if (rank_g_svr_load_def(rank_g_svr, rank_def) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load rank-def fail.", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    if (rank_g_svr_load_record(rank_g_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: init record fail.", gd_app_module_name(module));
        rank_g_svr_free(rank_g_svr);
        return -1;
    }

    /*没有任何记录，尝试自动从初始化数据中构建 */
    if (aom_obj_mgr_allocked_obj_count(rank_g_svr->m_record_mgr) == 0) {
        CPE_INFO(gd_app_em(app), "%s: create: no record, begin init", gd_app_module_name(module));
        if (rank_g_svr_load_init_records(rank_g_svr) != 0) {
            CPE_ERROR(gd_app_em(app),  "%s: create: load init records fail!", gd_app_module_name(module));
            return -1;
        }
    }
    
    if (rank_g_svr->m_debug) {
        rank_g_svr_index_t index;
        
        CPE_INFO(
            gd_app_em(app), "%s: create: done. meta=%s, key=%s, record-count=%d",
            gd_app_module_name(module), dr_meta_name(rank_g_svr->m_record_meta), dr_entry_name(rank_g_svr->m_uin_entry),
            aom_obj_mgr_allocked_obj_count(rank_g_svr->m_record_mgr));


        TAILQ_FOREACH(index, &rank_g_svr->m_indexs, m_next) {
            rank_g_svr_season_info_t season_info;
            
            CPE_INFO(
                rank_g_svr->m_em, "%s: index %d: record-season=%d, record-count=%d, recouse-season=%d, seasion-keep-count=%d, cur-season=%d",
                rank_g_svr_name(rank_g_svr), index->m_id,
                index->m_record_season, rt_size(index->m_rank_tree),
                index->m_season_use, index->m_season_keep_count,
                index->m_season_cur ? index->m_season_cur->m_season_id : 0);

            TAILQ_FOREACH(season_info,  &index->m_season_infos, m_next) {
                char buf1[64], buf2[64];
                CPE_INFO(
                    rank_g_svr->m_em, "%s:     %s seasion %d: %s ~ %s", rank_g_svr_name(rank_g_svr),
                    season_info == index->m_season_cur ? "[*]" : "   ",
                    season_info->m_season_id,
                    time_to_str(season_info->m_begin_time, buf1, sizeof(buf1)),
                    time_to_str(season_info->m_end_time, buf2, sizeof(buf2)));
            }
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void rank_g_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    rank_g_svr_t rank_g_svr;

    rank_g_svr = rank_g_svr_find_nc(app, gd_app_module_name(module));
    if (rank_g_svr) {
        rank_g_svr_free(rank_g_svr);
    }
}

static LPDRMETA rank_g_svr_load_meta(rank_g_svr_t svr, const char * str_meta) {
    dr_store_manage_t store_mgr;
    dr_store_t store;
    char const * sep;
    char lib_name[64];
    LPDRMETA meta;

    store_mgr = dr_store_manage_find_nc(svr->m_app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: store_mgr not exist!", rank_g_svr_name(svr));
        return NULL;
    }

    sep = strchr(str_meta, '.');
    if (sep == NULL || (sep - str_meta) > (sizeof(lib_name) - 1)) {
        CPE_ERROR(svr->m_em, "%s: pkg-meta %s format error or overflow!", rank_g_svr_name(svr), str_meta);
        return NULL;
    }
    memcpy(lib_name, str_meta, sep - str_meta);
    lib_name[sep - str_meta] = 0;

    store = dr_store_find(store_mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s not exist in %s!",
            rank_g_svr_name(svr), lib_name, dr_store_manage_name(store_mgr));
        return NULL;
    }

    meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s have no meta %s!",
            rank_g_svr_name(svr), lib_name, sep + 1);
        return NULL;
    }

    return meta;
}

static int rank_g_svr_load_def(rank_g_svr_t svr, cfg_t cfg) {
    const char * str_meta;
    LPDRMETA meta;
    struct cfg_it indexes_it;
    cfg_t index_cfg;
    
    str_meta = cfg_get_string(cfg, "meta", NULL);
    if (str_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: rank-def.meta not configured.", rank_g_svr_name(svr));
        return -1;
    }

    meta = rank_g_svr_load_meta(svr, str_meta);
    if (meta == NULL) return -1;
    if (rank_g_svr_set_record_meta(svr, meta) != 0) return -1;
    
    cfg_it_init(&indexes_it, cfg_find_cfg(cfg, "indexes"));
    while((index_cfg = cfg_it_next(&indexes_it))) {
        rank_g_svr_index_t index;
        uint16_t id;
        const char * attr;
        cfg_t season_cfg;
        
        if (cfg_try_get_uint16(index_cfg, "id", &id) != 0) {
            CPE_ERROR(svr->m_em, "%s: create: create index info: id not configured", rank_g_svr_name(svr));
            return -1;
        }

        attr = cfg_get_string(index_cfg, "attr", NULL);
        if (attr == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: create index info %d: attr not configured", rank_g_svr_name(svr), id);
            return -1;
        }

        index = rank_g_svr_index_create(svr, id, attr);
        if (index == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: create index info %d fail", rank_g_svr_name(svr), id);
            return -1;
        }

        if ((season_cfg = cfg_find_cfg(index_cfg, "season"))) {
            if (rank_g_svr_load_seasons(svr, index, season_cfg) != 0) return -1;
        }
    }

    return 0;
}

static int rank_g_svr_load_record(rank_g_svr_t svr) {
    const char * str_record_count;
    uint32_t record_count;
    const char * str_bucket_ratio;
    float bucket_ratio = 1.5;
    rank_g_svr_index_t index;

    str_record_count = gd_app_arg_find(svr->m_app, "--record-count");
    if (str_record_count == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: record init: record-count not conrigured", rank_g_svr_name(svr));
        return -1;
    }

    record_count = strtol(str_record_count, NULL, 10);

    if ((str_bucket_ratio = gd_app_arg_find(svr->m_app, "--bucket-ratio"))) {
        bucket_ratio = strtof(str_bucket_ratio, NULL);
    }

    if (rank_g_svr_record_init(svr, record_count, bucket_ratio) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: record init: init fail, record_count=%d", rank_g_svr_name(svr), record_count);
        return -1;
    }

    /*初始化index */
    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        if (rank_g_svr_index_init_record(index, record_count) != 0) {
            CPE_ERROR(svr->m_em,  "%s: record init: index %d init fail!", rank_g_svr_name(svr), index->m_id);
            return -1;
        }
    }
    
    return 0;
}

static int rank_g_svr_season_info_cmp_by_time(rank_g_svr_season_info_t l, rank_g_svr_season_info_t r, void * p) {
    if (l->m_begin_time == r->m_begin_time) return 0;
    return l->m_begin_time < r->m_begin_time ? -1 : 1;
}

static int rank_g_svr_load_seasons(rank_g_svr_t svr, rank_g_svr_index_t index, cfg_t cfg) {
    struct cfg_it season_it;
    cfg_t season_cfg;
    char seasions_path[64];
    uint16_t season_keep_count;
    char const * season_attr;
    rank_g_svr_season_info_t season_info, next_season_info;
    uint32_t cur_time;
    
    season_keep_count = cfg_get_uint16(cfg, "keep-count", 100);
    season_attr = cfg_get_string(cfg, "attr", NULL);
    if (season_attr == NULL) {
        CPE_ERROR(svr->m_em, "%s: index %d: seasion attr not configured", rank_g_svr_name(svr), index->m_id);
        return -1;
    }

    if (rank_g_svr_index_set_season_info(index, season_keep_count, season_attr) != 0) return -1;
    
    snprintf(seasions_path, sizeof(seasions_path), "seasions.%d", index->m_id);
    cfg_it_init(&season_it, cfg_find_cfg(gd_app_cfg(svr->m_app), seasions_path));
    
    while((season_cfg = cfg_it_next(&season_it))) {
        uint16_t season_id = cfg_get_uint16(season_cfg, "id", 0);
        uint32_t begin_time;
        uint32_t end_time;
        const char * str_value;

        if (season_id == 0) {
            CPE_INFO(svr->m_em, "%s: index %d: season id not configured", rank_g_svr_name(svr), index->m_id);
            return -1;
        }

        if ((str_value = cfg_get_string(season_cfg, "begin-time", NULL))) {
            begin_time = time_from_str(str_value);
            if (begin_time == 0) {
                CPE_INFO(svr->m_em, "%s: index %d: season %d begin time %s format error", rank_g_svr_name(svr), index->m_id, season_id, str_value);
                return -1;
            }
        }
        else {
            CPE_INFO(svr->m_em, "%s: index %d: season %d begin time not configured", rank_g_svr_name(svr), index->m_id, season_id);
            return -1;
        }            

        if ((str_value = cfg_get_string(season_cfg, "end-time", NULL))) {
            end_time = time_from_str(str_value);
            if (end_time == 0) {
                CPE_INFO(svr->m_em, "%s: index %d: season %d end time %s format error", rank_g_svr_name(svr), index->m_id, season_id, str_value);
                return -1;
            }

            if (end_time < begin_time) {
                CPE_INFO(svr->m_em, "%s: index %d: season %d end time %d overflow, begin time is %d", rank_g_svr_name(svr), index->m_id, season_id, end_time, begin_time);
                return -1;
            }
        }
        else {
            end_time = 0;
        }

        if (rank_g_svr_season_info_create(index, season_id, begin_time, end_time) == NULL) {
            CPE_INFO(svr->m_em, "%s: index %d: season %d create fail", rank_g_svr_name(svr), index->m_id, season_id);
            return -1;
        }
    }

    TAILQ_SORT(
        &index->m_season_infos, rank_g_svr_season_info, rank_g_svr_season_info_list, m_next,
        rank_g_svr_season_info_cmp_by_time, NULL);

    for(season_info = TAILQ_FIRST(&index->m_season_infos); season_info; season_info = next_season_info) {
        next_season_info = TAILQ_NEXT(season_info, m_next);
        if (next_season_info == NULL) {
            if (season_info->m_end_time == 0) {
                CPE_INFO(svr->m_em, "%s: index %d: season %d no end time, skip", rank_g_svr_name(svr), index->m_id, season_info->m_season_id);
                rank_g_svr_season_info_free(season_info);
            }

            break;
        }
        else {
            if (season_info->m_end_time == 0) {
                season_info->m_end_time = next_season_info->m_begin_time;
            }
            else if (season_info->m_end_time > next_season_info->m_begin_time) {
                CPE_ERROR(
                    svr->m_em, "%s: index %d: season %d end time %d overflow, next seasion begin time is %d",
                    rank_g_svr_name(svr), index->m_id, season_info->m_season_id, season_info->m_begin_time, next_season_info->m_end_time);
                return -1;
            }
        }
    }

    cur_time = rank_g_svr_cur_time(svr);
    
    TAILQ_FOREACH(season_info, &index->m_season_infos, m_next) {
        if (cur_time >= season_info->m_begin_time) {
            if (cur_time < season_info->m_end_time) {
                index->m_season_cur = season_info;
                break;
            }
        }
        else {
            index->m_season_cur = season_info;
            break;
        }
    }
    
    return 0;
}
