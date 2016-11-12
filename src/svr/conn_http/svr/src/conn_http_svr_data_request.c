#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "conn_http_svr_ops.h" 

static void conn_http_request_on_complete(ebb_request * ebb_request);
static void conn_http_request_on_path(ebb_request * ebb_request, const char *at, size_t length);
static void conn_http_request_on_query_string(ebb_request * ebb_request, const char *at, size_t length);
static void conn_http_request_on_fragment(ebb_request * ebb_request, const char *at, size_t length);
static void conn_http_request_on_body(ebb_request * ebb_request, const char *at, size_t length);

conn_http_request_t
conn_http_request_create(conn_http_connection_t connection) {
    conn_http_svr_t svr = connection->m_svr;
    conn_http_request_t request;

    request = mem_alloc(svr->m_alloc, sizeof(struct conn_http_request));
    if (request == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request ??? at connection %d: create: alloc fail",
            conn_http_svr_name(svr), connection->m_id);
        return NULL;
    }

    request->m_id = ++svr->m_max_request_id;
    request->m_state = conn_http_request_init;
    request->m_connection = connection;
    request->m_cmd = NULL;
    request->m_req_blk = NULL;
    request->m_res_blk = NULL;

    ebb_request_init(&request->m_ebb_request);
    request->m_ebb_request.data = request;
    request->m_ebb_request.on_complete = conn_http_request_on_complete;
    request->m_ebb_request.on_path = conn_http_request_on_path;
    request->m_ebb_request.on_query_string = conn_http_request_on_query_string;
    request->m_ebb_request.on_fragment = conn_http_request_on_fragment;
    request->m_ebb_request.on_body = conn_http_request_on_body;

    cpe_hash_entry_init(&request->m_hh_for_svr);
    if (cpe_hash_table_insert_unique(&svr->m_requests, request) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: create: insert fail!",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        mem_free(svr->m_alloc, request);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&connection->m_requests, request, m_next_for_connection);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: create",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
    }

    return request;
}

void conn_http_request_free(conn_http_request_t request) {
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;

    if (request->m_req_blk) {
        ringbuffer_free(svr->m_ringbuf, request->m_req_blk);
        request->m_req_blk = NULL;
    }

    if (request->m_res_blk) {
        ringbuffer_free(svr->m_ringbuf, request->m_res_blk);
        request->m_res_blk = NULL;
    }

    cpe_hash_table_remove_by_ins(&svr->m_requests, request);
    TAILQ_REMOVE(&connection->m_requests, request, m_next_for_connection);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: request %d at connection %d: free", conn_http_svr_name(svr), request->m_id, connection->m_id);
    }

    mem_free(svr->m_alloc, request);
}

void conn_http_request_free_all(conn_http_connection_t connection) {
    while(!TAILQ_EMPTY(&connection->m_requests)) {
        conn_http_request_free(TAILQ_FIRST(&connection->m_requests));
    }
}

conn_http_request_t
conn_http_request_find(conn_http_svr_t svr, uint32_t id) {
    struct conn_http_request key;
    key.m_id = id;

    return cpe_hash_table_find(&svr->m_requests, &key);
}

uint32_t conn_http_request_hash(conn_http_request_t request) {
    return request->m_id;
}

int conn_http_request_eq(conn_http_request_t l, conn_http_request_t r) {
    return l->m_id == r->m_id;
}

static void conn_http_request_on_path(ebb_request * ebb_request, const char *at, size_t length) {
    conn_http_request_t request = ebb_request->data;
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;
    conn_http_service_t service;
    char buf[length + 1];
    char * service_path;
    char * cmd_path;

    if (request->m_state != conn_http_request_init) return;

    memcpy(buf, at, length);
    buf[length] = 0;

    service_path = buf;
    if (*service_path == '/') service_path++;

    cmd_path = strchr(service_path, '/');
    if (cmd_path) {
        *cmd_path = 0;
        cmd_path++;
    }
    else {
        cmd_path = "";
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on path(%s - %s)", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, service_path, cmd_path);
    }

    service = conn_http_service_find(svr, service_path);
    if (service == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on path(%s - %s): can`t find service", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, service_path, cmd_path);
        conn_http_request_set_error(request, 404, "Not Found");
        return;
    }

    request->m_cmd = conn_http_cmd_find(service, cmd_path);
    if (request->m_cmd == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on path(%s - %s): can`t find cmd in service", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, service_path, cmd_path);
        conn_http_request_set_error(request, 404, "Not Found");
        return;
    }
}

static void conn_http_request_on_query_string(ebb_request * ebb_request, const char *at, size_t length) {
    conn_http_request_t request = ebb_request->data;
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;

    if (request->m_state != conn_http_request_init) return;

    if (svr->m_debug) {
        char buf[length + 1];
        memcpy(buf, at, length);
        buf[length] = 0;

        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on query_string: %s", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, buf);
    }
}

static void conn_http_request_on_fragment(ebb_request * ebb_request, const char *at, size_t length) {
    conn_http_request_t request = ebb_request->data;
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;

    if (svr->m_debug) {
        char buf[length + 1];
        memcpy(buf, at, length);
        buf[length] = 0;

        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on fragment: %s", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, buf);
    }
}

static void conn_http_request_on_body(ebb_request * ebb_request, const char *at, size_t length) {
    conn_http_request_t request = ebb_request->data;
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;
    conn_http_service_t service;

    if (request->m_state != conn_http_request_init) return;

    if (svr->m_debug) {
        char buf[length + 1];
        memcpy(buf, at, length);
        buf[length] = 0;

        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on body: %s", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, buf);
    }

    if (request->m_cmd == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on body: cmd not exist", 
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        conn_http_request_set_error(request, 404, "Not Found");
        return;
    }

    service = request->m_cmd->m_service;

    if (request->m_cmd->m_req_meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on body: path (%s - %s) no req_meta", 
            conn_http_svr_name(svr), request->m_id, connection->m_id, 
            service->m_path, request->m_cmd->m_path);
        conn_http_request_set_error(request, 500, "Internal Server Error");
        return;
    }

    service->m_formator->on_request(request, at, length);
}

static void conn_http_request_on_complete(ebb_request * ebb_request) {
    conn_http_request_t request = ebb_request->data;
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;

    if (request->m_state != conn_http_request_init) return;

    if (request->m_cmd == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on complete: no cmd!", 
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        conn_http_request_set_error(request, 404, "Not Found");
        return;
    }

    if (request->m_req_blk == NULL) {
        if (request->m_cmd->m_req_meta != NULL) {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: on complete: no request data!", 
                conn_http_svr_name(svr), request->m_id, connection->m_id);
            conn_http_request_set_error(request, 400, "Bad Request");
            return;
        }
        else {
            if (set_svr_stub_send_req_cmd(
                    svr->m_stub, set_svr_svr_info_svr_type_id(request->m_cmd->m_service->m_dispatch_to), 0,
                    request->m_id, request->m_cmd->m_req_id,
                    NULL, 0) < 0)
            {
                CPE_ERROR(
                    svr->m_em, "%s: request %d at connection %d: on complete: send cmd %d fail!", 
                    conn_http_svr_name(svr), request->m_id, connection->m_id, request->m_cmd->m_req_id);
                conn_http_request_set_error(request, 500, "Internal Server Error");
                return;
            }
            else {
                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: request %d at connection %d: on complete: send cmd %d!", 
                        conn_http_svr_name(svr), request->m_id, connection->m_id, request->m_cmd->m_req_id);
                }
            }
        }
    }
    else {
        int data_len = ringbuffer_block_total_len(svr->m_ringbuf, request->m_req_blk);
        void * data;
        if (request->m_req_blk->next >= 0) {
            ringbuffer_block_t new_blk;

            if (conn_http_request_alloc(&new_blk, svr, request, data_len) != 0) return;
            assert(new_blk);

            data = ringbuffer_copy(svr->m_ringbuf, request->m_req_blk, 0, new_blk);
            assert(data);

            ringbuffer_free(svr->m_ringbuf, request->m_req_blk);
            request->m_req_blk = new_blk;
        }
        else {
            ringbuffer_block_data(svr->m_ringbuf, request->m_req_blk, 0, &data);
            assert(data);
        }

        assert(request->m_cmd->m_req_meta);

        if (set_svr_stub_send_req_data(
                svr->m_stub, set_svr_svr_info_svr_type_id(request->m_cmd->m_service->m_dispatch_to), 0,
                request->m_id, data, data_len, request->m_cmd->m_req_meta,
                NULL, 0) < 0)
        {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: on complete: send data %s(len=%d) fail!",
                conn_http_svr_name(svr), request->m_id, connection->m_id, dr_meta_name(request->m_cmd->m_req_meta), data_len);
            conn_http_request_set_error(request, 500, "Internal Server Error");
            return;
        }
        else {
            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: request %d at connection %d: on complete: send data %s(len=%d) success!",
                    conn_http_svr_name(svr), request->m_id, connection->m_id, dr_meta_name(request->m_cmd->m_req_meta), data_len);
            }
        }

        /*free request buf after send*/
        ringbuffer_free(svr->m_ringbuf, request->m_req_blk);
        request->m_req_blk = NULL;

        request->m_state = conn_http_request_runing;
    }
}

void conn_http_request_link_node_r(conn_http_request_t request, ringbuffer_block_t blk) {
    if (request->m_req_blk) {
		ringbuffer_link(request->m_connection->m_svr->m_ringbuf, request->m_req_blk , blk);
	}
    else {
		blk->id = request->m_id;
		request->m_req_blk = blk;
	}
}

void conn_http_request_link_node_w(conn_http_request_t request, ringbuffer_block_t blk) {
    if (request->m_res_blk) {
		ringbuffer_link(request->m_connection->m_svr->m_ringbuf, request->m_res_blk , blk);
	}
    else {
		blk->id = request->m_id;
		request->m_res_blk = blk;
	}
}

int conn_http_request_alloc(ringbuffer_block_t * result, conn_http_svr_t svr, conn_http_request_t request, size_t size) {
    conn_http_connection_t connection = request->m_connection;
    ringbuffer_block_t blk;

    blk = ringbuffer_alloc(svr->m_ringbuf , size);
    while (blk == NULL) {
        conn_http_request_t free_request;
        conn_http_connection_t free_connection;
        int collect_id = ringbuffer_collect(svr->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: alloc: not enouth capacity, len=%d!",
                conn_http_svr_name(svr), request->m_id, connection->m_id, (int)size);
            ebb_connection_schedule_close(&connection->m_ebb_conn);
            return -1;
        }

        free_request = conn_http_request_find(svr, collect_id);
        assert(free_request);
        free_connection = free_request->m_connection;
        assert(free_connection);

        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: alloc: not enouth free buff, free connection %d!",
            conn_http_svr_name(svr), request->m_id, connection->m_id, free_connection->m_id);
        ebb_connection_schedule_close(&free_connection->m_ebb_conn);
        if (free_connection == connection) return -1;

        blk = ringbuffer_alloc(svr->m_ringbuf , size);
    }

    *result = blk;
    return 0;
}

void conn_http_request_set_error(conn_http_request_t request, uint32_t http_errno, const char * http_errmsg) {
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;
    int data_len = 128;
    void * data;

    if (conn_http_request_alloc(&blk, svr, request, data_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_error: alloc buf error!",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    if (ringbuffer_block_data(svr->m_ringbuf, blk, 0, &data) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_error: get ringbuf data fail!",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    data_len = snprintf(data, data_len, "HTTP/1.1 %d %s\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n", http_errno, http_errmsg);

    ringbuffer_shrink(svr->m_ringbuf, blk, data_len);

    conn_http_request_link_node_w(request, blk);

    request->m_state = conn_http_request_complete;

    conn_http_connection_check_send_response(connection);
}

void conn_http_request_set_response(
    conn_http_request_t request, const char * body_format, ringbuffer_block_t body_data, uint32_t body_size)
{
    conn_http_connection_t connection = request->m_connection;
    conn_http_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;
    ringbuffer_block_t link_to;
    int data_len = 128;
    void * data;

    if (conn_http_request_alloc(&blk, svr, request, data_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_response: alloc buf error!",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    if (ringbuffer_block_data(svr->m_ringbuf, blk, 0, &data) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_response: get ringbuf data fail!",
            conn_http_svr_name(svr), request->m_id, connection->m_id);
        ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    data_len = snprintf(data, data_len, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", body_format, body_size);

    ringbuffer_shrink(svr->m_ringbuf, blk, data_len);

    conn_http_request_link_node_w(request, blk);
    link_to = request->m_res_blk;
    assert(link_to);

    while(body_data && body_size > 0) {
        int blk_size;

        blk = ringbuffer_unlink(svr->m_ringbuf, &body_data);
        blk_size = ringbuffer_block_len(svr->m_ringbuf, blk, 0);

        if (blk_size > body_size) {
            ringbuffer_shrink(svr->m_ringbuf, blk, body_size);
            blk_size = body_size;
        }

        ringbuffer_link(svr->m_ringbuf, link_to, blk);
        link_to = blk;
        body_size -= blk_size;
    }

    request->m_state = conn_http_request_complete;

    conn_http_connection_check_send_response(connection);
}
