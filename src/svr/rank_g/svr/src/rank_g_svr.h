#ifndef SVR_RANK_G_SVR_TYPES_H
#define SVR_RANK_G_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/mongo_use_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/rank_g/svr_rank_g_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rank_g_svr * rank_g_svr_t;
typedef struct rank_g_svr_index * rank_g_svr_index_t;
typedef struct rank_g_svr_season_info * rank_g_svr_season_info_t;
typedef struct rt * rt_t;
typedef struct rt_node * rt_node_t;

typedef TAILQ_HEAD(rank_g_svr_index_list, rank_g_svr_index) rank_g_svr_index_list_t;
typedef TAILQ_HEAD(rank_g_svr_season_info_list, rank_g_svr_season_info) rank_g_svr_season_info_list_t;
    
typedef int (*rank_g_svr_sort_fun_t)(void const * l, void const * r);

struct rank_g_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    set_logic_sp_t m_set_sp;
    logic_op_register_t m_op_register;
    set_logic_rsp_manage_t m_rsp_manage;
    mongo_cli_proxy_t m_db;
    
    LPDRMETA m_pkg_meta_res_error;
    LPDRMETA m_pkg_meta_res_query;
    LPDRMETA m_pkg_meta_res_query_with_data;
    LPDRMETA m_pkg_meta_res_query_season;
    LPDRMETA m_meta_season_saving;
    LPDRMETA m_meta_season_role_to_rank;
    
    gd_timer_id_t m_check_timer_id;

    /*配置信息 */
    uint8_t m_index_count;
    rank_g_svr_index_list_t m_indexs;
    uint32_t m_result_count_limit;
    
    /*record数据 */
    LPDRMETA m_record_meta;
    uint32_t m_record_size;
    LPDRMETAENTRY m_uin_entry;
    uint32_t m_uin_start_pos;
    aom_obj_mgr_t m_record_mgr;
    aom_obj_hash_table_t m_record_hash;

    /*赛季保持正在执行过程中 */
    uint32_t m_saving_max_pkg_count;
    logic_context_id_t m_saving_op;
    uint32_t m_saving_error_time;
    
    /*db相关记录数据 */
    struct mem_buffer m_gen_metalib;
    LPDRMETA m_season_record_meta;
    uint32_t m_season_record_size;
    LPDRMETA m_season_record_list_meta;
    uint32_t m_season_record_data_start_pos;
    LPDRMETAENTRY m_season_record_list_count_entry;
    LPDRMETAENTRY m_season_record_list_data_entry;

    /**/
    struct mem_buffer m_record_buff;
    struct mem_buffer m_dump_buff;
};

typedef void (*rank_g_svr_op_t)(rank_g_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

/*operations of rank_g_svr */
rank_g_svr_t
rank_g_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em);

void rank_g_svr_free(rank_g_svr_t svr);

rank_g_svr_t rank_g_svr_find(gd_app_context_t app, cpe_hash_string_t name);
rank_g_svr_t rank_g_svr_find_nc(gd_app_context_t app, const char * name);
const char * rank_g_svr_name(rank_g_svr_t svr);
uint32_t rank_g_svr_cur_time(rank_g_svr_t svr);

int rank_g_svr_set_request_recv_at(rank_g_svr_t svr, const char * name);
int rank_g_svr_set_check_span(rank_g_svr_t svr, uint32_t span_ms);

int rank_g_svr_record_init(rank_g_svr_t svr, uint32_t record_count, float bucket_ratio);

const char * rank_g_svr_record_dump(rank_g_svr_t svr, void const * record);
void * rank_g_svr_record_buf(rank_g_svr_t svr, size_t capacity);

ptr_int_t rank_g_svr_tick(void * ctx, ptr_int_t arg, float delta_s);

int rank_g_svr_set_record_meta(rank_g_svr_t svr, LPDRMETA meta);

int rank_g_svr_load_init_records(rank_g_svr_t svr);

#ifdef __cplusplus
}
#endif

#endif
