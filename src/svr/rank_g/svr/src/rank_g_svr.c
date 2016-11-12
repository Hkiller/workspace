#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "usf/logic/logic_context.h"
#include "usf/logic_use/logic_op_register.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_stub_buff.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"

extern char g_metalib_svr_rank_g_pro[];
static void rank_g_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_rank_g_svr = {
    "svr_rank_g_svr",
    rank_g_svr_clear
};

struct logic_op_register_def g_rank_g_ops[] = {
    { "rank_g_op_update", rank_g_svr_op_update_send, rank_g_svr_op_update_recv }
    , { "rank_g_op_remove", rank_g_svr_op_remove_send, rank_g_svr_op_remove_recv }
    , { "rank_g_op_query", rank_g_svr_op_query_send, rank_g_svr_op_query_recv }
    , { "rank_g_op_query_with_data", rank_g_svr_op_query_with_data_send, rank_g_svr_op_query_with_data_recv }
    , { "rank_g_op_query_season", rank_g_svr_op_query_season_send, NULL }
    , { "rank_g_op_change_season", rank_g_svr_op_change_season_send, rank_g_svr_op_change_season_recv }
    , { "rank_g_op_dump", rank_g_svr_op_dump_send, NULL }
    , { "rank_g_op_init", rank_g_svr_op_init_send, NULL }
};

#define RANK_G_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_rank_g_pro, __name); \
    assert(svr-> __arg)

rank_g_svr_t
rank_g_svr_create(
    gd_app_context_t app, const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc, error_monitor_t em)
{
    struct rank_g_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct rank_g_svr));
    if (svr_node == NULL) return NULL;

    svr = (rank_g_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_set_sp = set_sp;
    svr->m_rsp_manage = rsp_manage;
    svr->m_db = db;
    svr->m_debug = 0;
    svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    svr->m_index_count = 0;
    svr->m_saving_max_pkg_count = 100;
    svr->m_saving_op = 0;
    svr->m_saving_error_time = 0;

    TAILQ_INIT(&svr->m_indexs);
    svr->m_result_count_limit = 1000;
    
    RANK_G_SVR_LOAD_META(m_pkg_meta_res_error, "svr_rank_g_res_error");
    RANK_G_SVR_LOAD_META(m_pkg_meta_res_query, "svr_rank_g_res_query");
    RANK_G_SVR_LOAD_META(m_pkg_meta_res_query_with_data, "svr_rank_g_res_query_with_data");
    RANK_G_SVR_LOAD_META(m_pkg_meta_res_query_season, "svr_rank_g_res_query_season");
    RANK_G_SVR_LOAD_META(m_meta_season_saving, "svr_rank_g_season_saving");
    RANK_G_SVR_LOAD_META(m_meta_season_role_to_rank, "svr_rank_g_season_role_to_rank");
    
    svr->m_record_meta = NULL;
    svr->m_record_size = 0;
    svr->m_uin_entry = NULL;
    svr->m_uin_start_pos = 0;
    svr->m_record_mgr = NULL;
    svr->m_record_hash = NULL;

    mem_buffer_init(&svr->m_gen_metalib, alloc);
    svr->m_season_record_meta = NULL;
    svr->m_season_record_size = 0;
    svr->m_season_record_list_meta = NULL;
    svr->m_season_record_list_count_entry = NULL;
    svr->m_season_record_list_data_entry = NULL;
    
    if (svr->m_rsp_manage) {
        svr->m_op_register = logic_op_register_create(app, NULL, alloc, em);
        if (svr->m_op_register == NULL) {
            CPE_ERROR(em, "%s: create: create op_register fail!", name);
            mem_buffer_clear(&svr->m_gen_metalib);
            nm_node_free(svr_node);
            return NULL;
        }

        if (logic_op_register_create_ops(
                svr->m_op_register,
                sizeof(g_rank_g_ops) / sizeof(g_rank_g_ops[0]),
                g_rank_g_ops,
                svr) != 0)
        {
            CPE_ERROR(em, "%s: create: register ops fail!", name);
            mem_buffer_clear(&svr->m_gen_metalib);
            logic_op_register_free(svr->m_op_register);
            nm_node_free(svr_node);
            return NULL;
        }
    }
    else {
        svr->m_op_register = NULL;
    }

    if (gd_app_tick_add(app, rank_g_svr_tick, svr, 0) != 0) {
        CPE_ERROR(em, "%s: create: add tick fail!", name);
        mem_buffer_clear(&svr->m_gen_metalib);
        if (svr->m_op_register) logic_op_register_free(svr->m_op_register);
        nm_node_free(svr_node);
        return NULL;
    }

    mem_buffer_init(&svr->m_record_buff, alloc);
    mem_buffer_init(&svr->m_dump_buff, alloc);
    
    nm_node_set_type(svr_node, &s_nm_node_type_rank_g_svr);

    return svr;
}

static void rank_g_svr_clear(nm_node_t node) {
    rank_g_svr_t svr;
    
    svr = (rank_g_svr_t)nm_node_data(node);

    gd_app_tick_remove(svr->m_app, rank_g_svr_tick, svr);

    if (svr->m_saving_op) {
        logic_context_t saving_ctx = logic_context_find(
            set_logic_rsp_manage_logic(svr->m_rsp_manage), svr->m_saving_op);
        if (saving_ctx) logic_context_free(saving_ctx);
        svr->m_saving_op = 0;
    }
    
    if (svr->m_op_register) {
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
    }
    
    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);
        assert(timer_mgr);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    /*index*/
    while(!TAILQ_EMPTY(&svr->m_indexs)) {
        rank_g_svr_index_free(TAILQ_FIRST(&svr->m_indexs));
    }
    assert(svr->m_index_count == 0);
    
    /*record*/
    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_hash) {
        aom_obj_hash_table_free(svr->m_record_hash);
        svr->m_record_hash = NULL;
    }

    mem_buffer_clear(&svr->m_gen_metalib);
    
    mem_buffer_clear(&svr->m_record_buff);
    mem_buffer_clear(&svr->m_dump_buff);
}

void rank_g_svr_free(rank_g_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_rank_g_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t rank_g_svr_app(rank_g_svr_t svr) {
    return svr->m_app;
}

rank_g_svr_t
rank_g_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_rank_g_svr) return NULL;
    return (rank_g_svr_t)nm_node_data(node);
}

rank_g_svr_t
rank_g_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_rank_g_svr) return NULL;
    return (rank_g_svr_t)nm_node_data(node);
}

const char * rank_g_svr_name(rank_g_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
rank_g_svr_name_hs(rank_g_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t rank_g_svr_cur_time(rank_g_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int rank_g_svr_record_init(rank_g_svr_t svr, uint32_t record_count, float bucket_ratio) {
    size_t record_buff_capacity;
    set_svr_stub_buff_t record_buff;
    size_t hash_table_buff_capacity;
    set_svr_stub_buff_t hash_table_buff;

    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_hash) {
        aom_obj_hash_table_free(svr->m_record_hash);
        svr->m_record_hash = NULL;
    }

    /*初始化记录数组 */
    if (aom_obj_mgr_buf_calc_capacity(&record_buff_capacity, svr->m_record_meta, record_count, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: record init: calc buf capacity fail. record_count=%d!",
            rank_g_svr_name(svr), record_count);
        return -1;
    }

    record_buff = set_svr_stub_buff_check_create(svr->m_stub, "record_buff", record_buff_capacity);
    if (record_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: record init: create record_buff fail, capacity=%d!",
            rank_g_svr_name(svr), (int)record_buff_capacity);
        return -1;
    }

    if (!set_svr_stub_buff_is_init(record_buff)) {
        if (aom_obj_mgr_buf_init(
                svr->m_record_meta,
                set_svr_stub_buff_data(record_buff), set_svr_stub_buff_capacity(record_buff), svr->m_em)
            != 0)
        {
            CPE_ERROR(svr->m_em,  "%s: record init: init record buf fail!", rank_g_svr_name(svr));
            return -1;
        }
        set_svr_stub_buff_set_init(record_buff, 1);
    }

    svr->m_record_mgr = aom_obj_mgr_create(svr->m_alloc, set_svr_stub_buff_data(record_buff), set_svr_stub_buff_capacity(record_buff), svr->m_em);
    if (svr->m_record_mgr == NULL) {
        CPE_ERROR(svr->m_em,  "%s: record init: create aom obj mgr fail!", rank_g_svr_name(svr));
        return -1;
    }

    /*初始化记录hash表 */
    hash_table_buff_capacity = aom_obj_hash_table_buf_calc_capacity(svr->m_record_mgr, bucket_ratio);

    hash_table_buff = set_svr_stub_buff_check_create(svr->m_stub, "hash_table_buff", hash_table_buff_capacity);
    if (hash_table_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: record init: create hash_table_buff fail, capacity=%d!",
            rank_g_svr_name(svr), (int)hash_table_buff_capacity);
        return -1;
    }

    if (!set_svr_stub_buff_is_init(hash_table_buff) || aom_obj_mgr_allocked_obj_count(svr->m_record_mgr) == 0) {
        if (aom_obj_hash_table_buf_init(
                svr->m_record_mgr, bucket_ratio, 
                dr_meta_key_hash,
                set_svr_stub_buff_data(hash_table_buff), set_svr_stub_buff_capacity(hash_table_buff),
                svr->m_em)
            != 0)
        {
            CPE_ERROR(svr->m_em,  "%s: record init: init hash table buff fail!", rank_g_svr_name(svr));
            return -1;
        }

        set_svr_stub_buff_set_init(hash_table_buff, 1);
    }
    
    svr->m_record_hash =
        aom_obj_hash_table_create(
            svr->m_alloc, svr->m_em,
            svr->m_record_mgr, dr_meta_key_hash, dr_meta_key_cmp,
            set_svr_stub_buff_data(hash_table_buff), set_svr_stub_buff_capacity(hash_table_buff));
    if (svr->m_record_hash == NULL) {
        CPE_ERROR(svr->m_em,  "%s: record init: create aom hash table fail!", rank_g_svr_name(svr));
        return -1;
    }
    
    CPE_INFO(
        svr->m_em,  "%s: record init: success, record-size=%d, record-count=%d, record-buf=%.2fm, hash-buf=%.2fm!",
        rank_g_svr_name(svr), (int)dr_meta_size(aom_obj_mgr_meta(svr->m_record_mgr)),
        aom_obj_mgr_allocked_obj_count(svr->m_record_mgr),
        record_buff_capacity / 1024.0 / 1024.0, hash_table_buff_capacity / 1024.0 / 1024.0);

    return 0;
}

void rank_g_svr_send_error_response(rank_g_svr_t svr, dp_req_t pkg_head, int16_t error) {
    if (set_pkg_sn(pkg_head)) {
        SVR_RANK_G_RES_ERROR pkg;
        pkg.error = error;

        if (set_svr_stub_send_response_data(
                svr->m_stub,
                set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
                &pkg, sizeof(pkg), svr->m_pkg_meta_res_error,
                NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: send error response: send pkg fail!", rank_g_svr_name(svr));
            return;
        }
    }
}

void * rank_g_svr_record_buf(rank_g_svr_t svr, size_t capacity) {
    if (capacity > mem_buffer_size(&svr->m_record_buff)) {
        mem_buffer_set_size(&svr->m_record_buff, capacity);
    }

    return mem_buffer_make_continuous(&svr->m_record_buff, 0);
}

const char * rank_g_svr_record_dump(rank_g_svr_t svr, void const * record) {
    return dr_json_dump_inline(&svr->m_dump_buff, record, svr->m_record_size, svr->m_record_meta);
}
