#ifndef SVR_CONN_HTTP_SVR_OPS_H
#define SVR_CONN_HTTP_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/conn_http/svr_conn_http_pro.h"
#include "conn_http_svr_types.h"

/*operations of conn_http_svr */
conn_http_svr_t
conn_http_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    uint16_t port,
    mem_allocrator_t alloc,
    error_monitor_t em);

void conn_http_svr_free(conn_http_svr_t svr);

conn_http_svr_t conn_http_svr_find(gd_app_context_t app, cpe_hash_string_t name);
conn_http_svr_t conn_http_svr_find_nc(gd_app_context_t app, const char * name);
const char * conn_http_svr_name(conn_http_svr_t svr);
int conn_http_svr_set_ringbuf_size(conn_http_svr_t svr, size_t capacity);

int conn_http_svr_set_request_recv_at(conn_http_svr_t svr, const char * name);
int conn_http_svr_set_response_recv_at(conn_http_svr_t svr, const char * name);

dp_req_t conn_http_svr_pkg_buf(conn_http_svr_t svr, size_t capacity);

/*service ops*/
conn_http_service_t
conn_http_service_create(conn_http_svr_t svr, const char * svr_path, set_svr_svr_info_t dispatch_to, conn_http_formator_t formator);
void conn_http_service_free(conn_http_service_t service);
void conn_http_service_free_all(conn_http_svr_t svr);
conn_http_service_t
conn_http_service_find(conn_http_svr_t svr, const char * path);

/*cmd ops*/
conn_http_cmd_t
conn_http_cmd_create(conn_http_service_t service, const char * path, uint32_t cmd, LPDRMETA req_meta);
void conn_http_cmd_free(conn_http_cmd_t cmd);
void conn_http_cmd_free_all(conn_http_service_t service);
conn_http_cmd_t
conn_http_cmd_find(conn_http_service_t service, const char * path);

/*connection ops*/
conn_http_connection_t
conn_http_connection_create(conn_http_svr_t svr);
void conn_http_connection_check_send_response(conn_http_connection_t connection);
void conn_http_connection_free(conn_http_connection_t connection);
void conn_http_connection_free_all(conn_http_svr_t svr);

/*connection ops*/
conn_http_request_t
conn_http_request_create(conn_http_connection_t connection);
void conn_http_request_free(conn_http_request_t request);
void conn_http_request_free_all(conn_http_connection_t connection);
conn_http_request_t conn_http_request_find(conn_http_svr_t svr, uint32_t id);
uint32_t conn_http_request_hash(conn_http_request_t request);
void conn_http_request_set_error(conn_http_request_t request, uint32_t http_errno, const char * http_errmsg);
void conn_http_request_set_response(
    conn_http_request_t request,
    const char * body_format, ringbuffer_block_t body_data, uint32_t body_size);

int conn_http_request_eq(conn_http_request_t l, conn_http_request_t r);
int conn_http_request_alloc(ringbuffer_block_t * result, conn_http_svr_t svr, conn_http_request_t request, size_t size);
void conn_http_request_link_node_r(conn_http_request_t request, ringbuffer_block_t blk);
void conn_http_request_link_node_w(conn_http_request_t request, ringbuffer_block_t blk);

/*formators*/
extern struct conn_http_formator g_conn_http_formator_json;
extern struct conn_http_formator g_conn_http_formator_yaml;
extern struct conn_http_formator g_conn_http_formator_xml;

/*conn_http ss ops*/
/*void conn_http_svr_op_response(conn_http_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head); */

#endif
