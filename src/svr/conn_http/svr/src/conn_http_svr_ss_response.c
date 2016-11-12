#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "conn_http_svr_ops.h"

int conn_http_response_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_http_svr_t svr = ctx;
    dp_req_t pkg_head;
    uint32_t sn;
    conn_http_request_t request;
    conn_http_connection_t connection;
    conn_http_service_t service;
    uint32_t r_cmd;
    LPDRMETA r_meta;
    void * r_data;
    size_t r_data_size;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process request: no pkg head!", conn_http_svr_name(svr));
        return -1;
    }

    sn = set_pkg_sn(pkg_head);
    if (sn == 0) {
        CPE_ERROR(svr->m_em, "%s: response: response no sn, skip!", conn_http_svr_name(svr));
        return -1;
    }

    request = conn_http_request_find(svr, sn);
    if (request == NULL) {
        CPE_ERROR(svr->m_em, "%s: response: request %d not exist!", conn_http_svr_name(svr), sn);
        return -1;
    }

    connection = request->m_connection;

    if (request->m_state != conn_http_request_runing) {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: state is not runing, skip response", 
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        return -1;
    }

    if (request->m_cmd == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: cmd not exist", 
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        conn_http_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    service = request->m_cmd->m_service;

    if (set_svr_stub_read_data(
            svr->m_stub, service->m_dispatch_to, req,
            &r_cmd, &r_meta, &r_data, &r_data_size)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: read response data fail", 
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        conn_http_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (r_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: read response data: cmd %d no data meta, can`t send back", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, r_cmd);
        conn_http_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (r_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: read response data: cmd %d no data, can`t send back", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, r_cmd);
        conn_http_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    service->m_formator->on_response(request, r_data, r_data_size, r_meta);

    request->m_state = conn_http_request_complete;

    conn_http_connection_check_send_response(request->m_connection);

    return 0;
}
