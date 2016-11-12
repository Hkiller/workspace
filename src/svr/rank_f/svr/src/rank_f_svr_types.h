#ifndef SVR_RANK_F_SVR_TYPES_H
#define SVR_RANK_F_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/rank_f/svr_rank_f_internal.h"

typedef struct rank_f_svr * rank_f_svr_t;
typedef struct rank_f_svr_index_info * rank_f_svr_index_info_t;
typedef struct rank_f_svr_index * rank_f_svr_index_t;
typedef struct rank_f_svr_index_buf * rank_f_svr_index_buf_t;

typedef TAILQ_HEAD(rank_f_svr_index_list, rank_f_svr_index) rank_f_svr_index_list_t;
typedef int (*rank_f_svr_sort_fun_t)(void const * l, void const * r);

struct rank_f_svr_index_sorter {
    LPDRMETAENTRY m_sort_entry;
    uint16_t m_data_start_pos;
    rank_f_svr_sort_fun_t m_sort_fun;
};
 
struct rank_f_svr_index_info {
    rank_f_svr_t m_svr;
    uint16_t m_id;
    uint16_t m_sorter_count;
    struct rank_f_svr_index_sorter m_sorters[3];
};

struct rank_f_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;
    uint32_t m_page_size;

    LPDRMETA m_pkg_meta_res_error;
    LPDRMETA m_pkg_meta_res_query;
    LPDRMETA m_pkg_meta_res_query_with_data;

    gd_timer_id_t m_check_timer_id;

    dp_rsp_t m_recv_at;

    /*配置信息 */
    LPDRMETA m_data_meta;
    uint32_t m_data_size;
    struct rank_f_svr_index_info m_index_infos[SVR_RANK_F_INDEX_MAX];

    /*record数据 */
    struct mem_buffer m_record_metalib;
    LPDRMETA m_record_meta;
    uint32_t m_record_size;
    aom_obj_mgr_t m_record_mgr;
    void * m_record_buf;

    /*index数据 */
    /*    统计信息 */
    uint64_t m_index_page_count;
    uint64_t m_index_count;
    uint64_t m_index_free_count;
    uint64_t m_index_using_count;
    /*    维护数据结构 */
    rank_f_svr_index_list_t m_indexes_for_check;
    struct cpe_hash_table m_indexes;
    rank_f_svr_index_list_t m_free_indexes;
    rank_f_svr_index_list_t m_index_heads;

    /*buf相关数据 */
    /*    统计信息 */
    uint64_t m_buf_page_count;
    uint64_t m_buf_count;
    uint64_t m_buf_free_count;
    uint64_t m_buf_using_count;
    /*    维护数据结构 */
    rank_f_svr_index_buf_t m_free_bufs;
    rank_f_svr_index_buf_t m_buf_heads;
};

struct rank_f_svr_index {
    uint64_t m_user_id;
    uint8_t m_index_id;
    uint16_t m_record_count;
    uint32_t m_last_op_time;
    struct rank_f_svr_index_buf * m_bufs;
    TAILQ_ENTRY(rank_f_svr_index) m_next;
    struct cpe_hash_entry m_hh;
};

#define RANK_F_SVR_INDEX_BUF_RECORD_COUNT 63
struct rank_f_svr_index_buf {
    struct rank_f_svr_index_buf * m_next;
    uint32_t m_record_count;
    void * m_records[RANK_F_SVR_INDEX_BUF_RECORD_COUNT];
};

typedef void (*rank_f_svr_op_t)(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
