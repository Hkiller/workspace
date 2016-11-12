#ifndef SVR_RANK_F_SVR_OPS_H
#define SVR_RANK_F_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "rank_f_svr_types.h"
#include "protocol/svr/rank_f/svr_rank_f_pro.h"

/*operations of rank_f_svr */
rank_f_svr_t
rank_f_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void rank_f_svr_free(rank_f_svr_t svr);

rank_f_svr_t rank_f_svr_find(gd_app_context_t app, cpe_hash_string_t name);
rank_f_svr_t rank_f_svr_find_nc(gd_app_context_t app, const char * name);
const char * rank_f_svr_name(rank_f_svr_t svr);
uint32_t rank_f_svr_cur_time(rank_f_svr_t svr);

int rank_f_svr_set_request_recv_at(rank_f_svr_t svr, const char * name);
int rank_f_svr_set_check_span(rank_f_svr_t svr, uint32_t span_ms);

int rank_f_svr_record_init_from_mem(rank_f_svr_t svr, size_t memory_size);
int rank_f_svr_record_init_from_shm(rank_f_svr_t svr, int shm_key);

void * rank_f_svr_make_response(dp_req_t * res_body, rank_f_svr_t svr, dp_req_t req_body, size_t more_capacity);

uint16_t rank_f_svr_gid_start_pos(rank_f_svr_t svr);

/*index_info*/
rank_f_svr_index_info_t
rank_f_svr_index_info_create(rank_f_svr_t svr, uint16_t id);
int rank_f_svr_index_info_add_sorter(
    rank_f_svr_index_info_t index_info, const char * entry_path, const char * order);

/*rank_f_svr_index*/
rank_f_svr_index_t rank_f_svr_index_alloc(rank_f_svr_t svr, uint64_t user_id, uint8_t index_id);
void rank_f_svr_index_clear_records(rank_f_svr_t svr, rank_f_svr_index_t index);
void rank_f_svr_index_destory_records(rank_f_svr_t svr, rank_f_svr_index_t index);
void rank_f_svr_index_free(rank_f_svr_t svr, rank_f_svr_index_t index);
void rank_f_svr_index_free_all(rank_f_svr_t svr);

rank_f_svr_index_t rank_f_svr_index_find(rank_f_svr_t svr, uint64_t user_id, uint8_t index_id);

uint32_t rank_f_svr_index_hash(rank_f_svr_index_t index);
int rank_f_svr_index_eq(rank_f_svr_index_t l, rank_f_svr_index_t r);

/*record*/
int rank_f_svr_record_update(rank_f_svr_t svr, rank_f_svr_index_t gid_index, void const * record);
int rank_f_svr_record_remove(rank_f_svr_t svr, rank_f_svr_index_t gid_index, void const * key);
int rank_f_svr_record_sort(rank_f_svr_t svr, rank_f_svr_index_t index, rank_f_svr_index_t records);


void * rank_f_svr_record_find(rank_f_svr_t svr, rank_f_svr_index_t index, void const * key);

/*rank_f_svr_index_buf*/
rank_f_svr_index_buf_t rank_f_svr_index_buf_alloc(rank_f_svr_t svr);
void rank_f_svr_index_buf_free(rank_f_svr_t svr, rank_f_svr_index_buf_t buf);
void rank_f_svr_index_buf_release_all(rank_f_svr_t svr);

/*和用户相关的操作 */
int rank_f_svr_user_clear_index(rank_f_svr_t svr, uint64_t user_id); /*将除了gid以外的index清理掉 */
int rank_f_svr_user_destory(rank_f_svr_t svr, uint64_t user_id);
rank_f_svr_index_t rank_f_svr_user_check_create(rank_f_svr_t svr, uint64_t user_id);
int rank_f_svr_user_index_check_create(rank_f_svr_index_t * r, rank_f_svr_t svr, uint64_t user_id, uint8_t indx_id);

/*rank_f request ops*/
void rank_f_svr_request_update(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_f_svr_request_remove(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_f_svr_request_clear(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_f_svr_request_query(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);
void rank_f_svr_request_query_with_data(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

/*protocol utils*/
void rank_f_svr_send_error_response(rank_f_svr_t svr, dp_req_t pkg_head, int16_t error);

#endif
