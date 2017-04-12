#ifndef SVR_CONN_SVR_OPS_H
#define SVR_CONN_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "conn_svr_types.h"
#include "protocol/svr/conn/svr_conn_pro.h"

/*operations of conn_svr */
conn_svr_t
conn_svr_create(
    gd_app_context_t app,
    const char * name,
    uint16_t conn_svr_type,
    mem_allocrator_t alloc, error_monitor_t em);

void conn_svr_free(conn_svr_t svr);

conn_svr_t conn_svr_find(gd_app_context_t app, cpe_hash_string_t name);
conn_svr_t conn_svr_find_nc(gd_app_context_t app, const char * name);
const char * conn_svr_name(conn_svr_t svr);
uint32_t conn_svr_cur_time(conn_svr_t svr);

int conn_svr_set_ss_send_to(conn_svr_t svr, const char * send_to);
int conn_svr_set_ss_request_recv_at(conn_svr_t svr, const char * name);
int conn_svr_set_ss_trans_recv_at(conn_svr_t svr, const char * name);
int conn_svr_set_check_span(conn_svr_t svr, uint32_t span_ms);
int conn_svr_set_ringbuf_size(conn_svr_t svr, size_t capacity);

int conn_svr_start(conn_svr_t svr, const char * ip, uint16_t port, int accept_queue_size);
void conn_svr_stop(conn_svr_t svr);

dp_req_t conn_svr_pkg_buf(conn_svr_t svr);
int conn_svr_send_pkg(conn_svr_t svr, dp_req_t req);

/*conn operations*/
conn_svr_conn_t conn_svr_conn_create(conn_svr_t svr, int fd, uint32_t conn_id);
void conn_svr_conn_free(conn_svr_conn_t conn);
void conn_svr_conn_free_all(conn_svr_t svr);

void conn_svr_conn_start_watch(conn_svr_conn_t conn);
conn_svr_conn_t conn_svr_conn_find_by_conn_id(conn_svr_t svr, uint32_t conn_id);
conn_svr_conn_t conn_svr_conn_find_by_user_id(conn_svr_t svr, uint64_t user_id);
void conn_svr_conn_link_node_r(conn_svr_conn_t conn, ringbuffer_block_t blk);
void conn_svr_conn_link_node_w(conn_svr_conn_t conn, ringbuffer_block_t blk);
void conn_svr_conn_update_op_time(conn_svr_conn_t conn);

uint32_t conn_svr_conn_conn_id_hash(conn_svr_conn_t conn);
int conn_svr_conn_conn_id_eq(conn_svr_conn_t l, conn_svr_conn_t r);

uint32_t conn_svr_conn_user_id_hash(conn_svr_conn_t conn);
int conn_svr_conn_user_id_eq(conn_svr_conn_t l, conn_svr_conn_t r);

int conn_svr_conn_set_user_info(conn_svr_conn_t conn, CONN_SVR_CONN_INFO const * data);

int conn_svr_conn_alloc(ringbuffer_block_t * result, conn_svr_t svr, conn_svr_conn_t conn, size_t size);
void conn_svr_conn_check_and_send(conn_svr_conn_t conn, dp_req_t pkg);
int conn_svr_conn_net_send(conn_svr_conn_t conn, uint16_t from_svr_type, int8_t err, uint32_t sn, void const * data, uint16_t data_len, LPDRMETA meta);

/*conn_svr_backend operations*/
conn_svr_backend_t
conn_svr_backend_create(conn_svr_t svr, uint16_t svr_type);
void conn_svr_backend_free(conn_svr_backend_t backend);
void conn_svr_backend_free_all(conn_svr_t svr);

conn_svr_backend_t conn_svr_backend_find(conn_svr_t svr, uint16_t svr_type);

uint32_t conn_svr_backend_hash(conn_svr_backend_t backend);
int conn_svr_backend_eq(conn_svr_backend_t l, conn_svr_backend_t r);

/*protocol process ops*/
void conn_svr_op_bind_user(conn_svr_t svr, dp_req_t pkg);
void conn_svr_op_close(conn_svr_t svr, dp_req_t pkg);

#endif
