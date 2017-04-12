#include <assert.h>
#include "payment_deliver_svr_connection.h"
#include "payment_deliver_svr_request.h"

static void payment_deliver_svr_on_close(ebb_connection * connection);
static ebb_request * payment_deliver_svr_new_request(ebb_connection *connection);

payment_deliver_connection_t
payment_deliver_connection_create(payment_deliver_svr_t svr) {
    payment_deliver_connection_t connection;

    connection = mem_alloc(svr->m_alloc, sizeof(struct payment_deliver_connection));
    if (connection == NULL) {
        CPE_ERROR(svr->m_em, "%s: connection: create: alloc fail", payment_deliver_svr_name(svr));
        return NULL;
    }

    connection->m_id = ++svr->m_max_conn_id;
    connection->m_svr = svr;

    ebb_connection_init(&connection->m_ebb_conn);
    connection->m_ebb_conn.data = connection;
    connection->m_ebb_conn.new_request = payment_deliver_svr_new_request;
    connection->m_ebb_conn.on_close = payment_deliver_svr_on_close;

    TAILQ_INIT(&connection->m_requests);

    TAILQ_INSERT_TAIL(&svr->m_connections, connection, m_next_for_svr);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: connection %d: create", payment_deliver_svr_name(svr), connection->m_id);
    }

    return connection;
}

void payment_deliver_connection_free(payment_deliver_connection_t connection) {
    payment_deliver_svr_t svr = connection->m_svr;

    payment_deliver_request_free_all(connection);

    TAILQ_REMOVE(&svr->m_connections, connection, m_next_for_svr);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: connection %d: free", payment_deliver_svr_name(svr), connection->m_id);
    }

    mem_free(svr->m_alloc, connection);
}

void payment_deliver_connection_free_all(payment_deliver_svr_t svr) {
    while(!TAILQ_EMPTY(&svr->m_connections)) {
        payment_deliver_connection_free(TAILQ_FIRST(&svr->m_connections));
    }
}

static void payment_deliver_svr_on_close(ebb_connection * ebb_conn) {
    payment_deliver_connection_t connection = ebb_conn->data;
    payment_deliver_svr_t svr = connection->m_svr;

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: connection %d: on close", payment_deliver_svr_name(svr), connection->m_id);
    }

    payment_deliver_connection_free(connection);
}

static ebb_request* payment_deliver_svr_new_request(ebb_connection * ebb_conn) {
    payment_deliver_connection_t connection = ebb_conn->data;
    payment_deliver_request_t request;

    request = payment_deliver_request_create(connection);
    if (request == NULL) {
        return NULL;
    }

    return &request->m_ebb_request;
}

static void payment_deliver_svr_continue_responding(ebb_connection * ebb_conn) {
    payment_deliver_connection_t connection = ebb_conn->data;
    payment_deliver_svr_t svr = connection->m_svr;
    payment_deliver_request_t request = TAILQ_FIRST(&connection->m_requests);
    void * data;
    int data_len;

    assert(request);

    data_len = ringbuffer_block_data(svr->m_ringbuf, request->m_res_blk, 0, &data);
    assert(data_len > 0);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: send %d data complete!", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, data_len);
    }

    assert(request->m_res_blk);
    request->m_res_blk = ringbuffer_yield(svr->m_ringbuf, request->m_res_blk, data_len);

    if (request->m_res_blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: keep-alive=%d!", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id,
            ebb_request_should_keep_alive(&request->m_ebb_request));
        
        if (!ebb_request_should_keep_alive(&request->m_ebb_request)) {

            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: request %d at connection %d: not keep-alive, close connection!", 
                    payment_deliver_svr_name(svr), request->m_id, connection->m_id);
            }

            ebb_connection_schedule_close(&connection->m_ebb_conn);
            return;
        }
        else {
            payment_deliver_request_free(request);
        }
    }

    payment_deliver_connection_check_send_response(connection);
}

void payment_deliver_connection_check_send_response(payment_deliver_connection_t connection) {
    payment_deliver_svr_t svr = connection->m_svr;
    payment_deliver_request_t request;
    void * data;
    int data_len;

    if(ev_is_active(&connection->m_ebb_conn.write_watcher)) return;

    while((request = TAILQ_FIRST(&connection->m_requests))) {
        if (request->m_state != payment_deliver_request_complete) break;

        if (request->m_res_blk == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: request completed, but no response!", 
                payment_deliver_svr_name(svr), request->m_id, connection->m_id);
            goto REQUEST_ERROR;
        }

        data_len = ringbuffer_block_data(svr->m_ringbuf, request->m_res_blk, 0, &data);
        if (data_len <= 0) {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: get response data fail, rv=%d!", 
                payment_deliver_svr_name(svr), request->m_id, connection->m_id, data_len);
            goto REQUEST_ERROR;
        }

        if (!ebb_connection_write(&connection->m_ebb_conn, data, data_len, payment_deliver_svr_continue_responding)) {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: send data fail, close connection!", 
                payment_deliver_svr_name(svr), request->m_id, connection->m_id);
            ebb_connection_schedule_close(&connection->m_ebb_conn);
            return;
        }

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: request %d at connection %d: send %d data begin!", 
                payment_deliver_svr_name(svr), request->m_id, connection->m_id, data_len);
        }

        return;
    REQUEST_ERROR:
        if (!ebb_request_should_keep_alive(&request->m_ebb_request)) {
            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: request %d at connection %d: not keep-alive, close connection!", 
                    payment_deliver_svr_name(svr), request->m_id, connection->m_id);
            }
            ebb_connection_schedule_close(&connection->m_ebb_conn);
            return;
        }
        else {
            payment_deliver_request_free(request);
            continue;
        }
        
    }
}
