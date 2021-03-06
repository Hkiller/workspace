#include <assert.h>
#include "cpe/utils/stream_ringbuffer.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_json.h"
#include "conn_http_svr_ops.h"

static ringbuffer_block_t on_json_request_buf_alloc(void * ctx) {
    conn_http_request_t request = ctx;
    conn_http_svr_t svr = request->m_connection->m_svr;
    ringbuffer_block_t blk;

    if (conn_http_request_alloc(&blk, svr, request, 1024) != 0) {
        return NULL;
    }
    else {
        return blk;
    }
}

static void on_json_request(conn_http_request_t request, const void * input_data, size_t input_data_size) {
    conn_http_connection_t connection = request->m_connection;
    conn_http_cmd_t cmd = request->m_cmd;
    conn_http_service_t service = cmd->m_service;
    conn_http_svr_t svr = service->m_svr;
    ringbuffer_block_t blk;
    void * data;
    char buf[input_data_size + 1];
    int data_size;

RETRY:
    if (conn_http_request_alloc(&blk, svr, request, cmd->m_pkg_buf_size) != 0) return;
    assert(blk);

    data = NULL;
    ringbuffer_data(svr->m_ringbuf, blk, cmd->m_pkg_buf_size, 0, (void*)&data);
    assert(data);

    assert(cmd->m_req_meta);

    memcpy(buf, input_data, input_data_size);
    buf[input_data_size] = 0;

    data_size = dr_json_read(data, cmd->m_pkg_buf_size, buf, cmd->m_req_meta, svr->m_em);
    if (data_size <= 0) {
        if (data_size == dr_code_error_not_enough_output) {
            uint32_t new_pkg_buf_size = cmd->m_pkg_buf_size < 64 ? 64 : cmd->m_pkg_buf_size * 2;
            
            CPE_INFO(
                svr->m_em, "%s: request %d at connection %d: on json request: resize pkg buf from %d to %d!",
                conn_http_svr_name(svr), request->m_id, connection->m_id, cmd->m_pkg_buf_size, new_pkg_buf_size);

            cmd->m_pkg_buf_size = new_pkg_buf_size;
            ringbuffer_free(svr->m_ringbuf, blk);
            goto RETRY;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: on json request: read from json fail, error=%d!",
                conn_http_svr_name(svr), request->m_id, connection->m_id, data_size);
            ringbuffer_free(svr->m_ringbuf, blk);
            conn_http_request_set_error(request, 400, "Bad Request");
            return;
        }
    }

    ringbuffer_shrink(svr->m_ringbuf, blk, data_size);
    conn_http_request_link_node_r(request, blk);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: read data from json success, data-size=%d!",
            conn_http_svr_name(svr), request->m_id, connection->m_id, data_size);
    }
}

static void on_json_response(conn_http_request_t request, const void * data, size_t data_len, LPDRMETA data_meta) {
    conn_http_connection_t connection = request->m_connection;
    conn_http_service_t service = request->m_cmd->m_service;
    conn_http_svr_t svr = service->m_svr;
    int result_size;
    struct write_stream_ringbuffer stream = CPE_WRITE_STREAM_RINGBUFFER_INITIALIZER(svr->m_ringbuf, on_json_request_buf_alloc, (void*)request);

    /*构造body */
    result_size = dr_json_print((write_stream_t)&stream, data, data_len, data_meta, DR_JSON_PRINT_MINIMIZE, svr->m_em);
    if (stream.m_error) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on json response: generate result ringbuf error!",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        if (stream.m_first_blk) ringbuffer_free(svr->m_ringbuf, stream.m_first_blk);
        return;
    }

    if (result_size < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on json response: write to json fail, error=%d!",
            conn_http_svr_name(svr), request->m_id, connection->m_id, result_size);
        if (stream.m_first_blk) ringbuffer_free(svr->m_ringbuf, stream.m_first_blk);
        conn_http_request_set_error(request, 400, "Bad Request");
        return;
    }

    conn_http_request_set_response(request, "text/json", &stream.m_first_blk, result_size);
    if (stream.m_first_blk) ringbuffer_free(svr->m_ringbuf, stream.m_first_blk);
}

struct conn_http_formator g_conn_http_formator_json = {
    on_json_request
    , on_json_response
};
