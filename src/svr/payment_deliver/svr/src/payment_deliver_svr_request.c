#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_deliver_svr_request.h"
#include "payment_deliver_svr_connection.h"
#include "payment_deliver_svr_adapter.h"
#include "payment_deliver_svr_adapter_type.h"

static void payment_deliver_request_on_complete(ebb_request * ebb_request);
static void payment_deliver_request_on_path(ebb_request * ebb_request, const char *at, size_t length);
static void payment_deliver_request_on_query_string(ebb_request * ebb_request, const char *at, size_t length);
static void payment_deliver_request_on_fragment(ebb_request * ebb_request, const char *at, size_t length);
static void payment_deliver_request_on_body(ebb_request * ebb_request, const char *at, size_t length);

payment_deliver_request_t
payment_deliver_request_create(payment_deliver_connection_t connection) {
    payment_deliver_svr_t svr = connection->m_svr;
    payment_deliver_request_t request;

    request = mem_alloc(svr->m_alloc, sizeof(struct payment_deliver_request));
    if (request == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request ??? at connection %d: create: alloc fail",
            payment_deliver_svr_name(svr), connection->m_id);
        return NULL;
    }

    request->m_id = ++svr->m_max_request_id;
    request->m_state = payment_deliver_request_init;
    request->m_connection = connection;
    request->m_adapter = NULL;
    request->m_req_blk = NULL;
    request->m_res_blk = NULL;

    ebb_request_init(&request->m_ebb_request);
    request->m_ebb_request.data = request;
    request->m_ebb_request.on_complete = payment_deliver_request_on_complete;
    request->m_ebb_request.on_path = payment_deliver_request_on_path;
    request->m_ebb_request.on_query_string = payment_deliver_request_on_query_string;
    request->m_ebb_request.on_fragment = payment_deliver_request_on_fragment;
    request->m_ebb_request.on_body = payment_deliver_request_on_body;

    cpe_hash_entry_init(&request->m_hh_for_svr);
    if (cpe_hash_table_insert_unique(&svr->m_requests, request) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: create: insert fail!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        mem_free(svr->m_alloc, request);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&connection->m_requests, request, m_next_for_connection);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: create",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
    }

    return request;
}

void payment_deliver_request_free(payment_deliver_request_t request) {
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;

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
        CPE_INFO(svr->m_em, "%s: request %d at connection %d: free", payment_deliver_svr_name(svr), request->m_id, connection->m_id);
    }

    mem_free(svr->m_alloc, request);
}

void payment_deliver_request_free_all(payment_deliver_connection_t connection) {
    while(!TAILQ_EMPTY(&connection->m_requests)) {
        payment_deliver_request_free(TAILQ_FIRST(&connection->m_requests));
    }
}

payment_deliver_request_t
payment_deliver_request_find(payment_deliver_svr_t svr, uint32_t id) {
    struct payment_deliver_request key;
    key.m_id = id;

    return cpe_hash_table_find(&svr->m_requests, &key);
}

void payment_deliver_request_data_clear(payment_deliver_request_t request) {
    if (request->m_req_blk) {
        ringbuffer_free(request->m_connection->m_svr->m_ringbuf, request->m_req_blk);
        request->m_req_blk = NULL;
    }
}

static char * payment_deliver_request_realloc_data(payment_deliver_svr_t svr, payment_deliver_request_t request, int data_len) {
    ringbuffer_block_t new_blk;
    char * data;
            
    if (payment_deliver_request_alloc(&new_blk, svr, request, data_len + 1) != 0) return NULL;
    assert(new_blk);

    data = ringbuffer_copy(svr->m_ringbuf, request->m_req_blk, 0, new_blk);
    assert(data);
    data[data_len] = 0;

    ringbuffer_free(svr->m_ringbuf, request->m_req_blk);
    request->m_req_blk = new_blk;

    return data;
}

char * payment_deliver_request_data(payment_deliver_request_t request) {
    payment_deliver_svr_t svr = request->m_connection->m_svr;
    
    if (svr->m_ringbuf == NULL) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: no ringbuffer", 
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        return NULL;
    }
    else if (request->m_req_blk == NULL) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: no request data", 
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        return NULL;
    }
    else {
        int data_len = ringbuffer_block_total_len(svr->m_ringbuf, request->m_req_blk);
        if (request->m_req_blk->next >= 0) {
            return payment_deliver_request_realloc_data(svr, request, data_len);
        }
        else {
            void * data;
            
            ringbuffer_block_data(svr->m_ringbuf, request->m_req_blk, 0, &data);
            assert(data);

            if (((char*)data)[data_len - 1] != 0) {
                return payment_deliver_request_realloc_data(svr, request, data_len);
            }
            else {
                return data;
            }
        }
    }
}

uint32_t payment_deliver_request_hash(payment_deliver_request_t request) {
    return request->m_id;
}

int payment_deliver_request_eq(payment_deliver_request_t l, payment_deliver_request_t r) {
    return l->m_id == r->m_id;
}

static void payment_deliver_request_on_path(ebb_request * ebb_request, const char *at, size_t length) {
    payment_deliver_request_t request = ebb_request->data;
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;
    char buf[length + 1];
    char * adapter_path;

    if (request->m_state != payment_deliver_request_init) return;

    memcpy(buf, at, length);
    buf[length] = 0;

    adapter_path = buf;
    if (*adapter_path == '/') adapter_path++;

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on path(%s)", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, adapter_path);
    }

    request->m_adapter = payment_deliver_adapter_find_by_name(svr, adapter_path);
    if (request->m_adapter == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on path(%s): can`t find adapter",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, adapter_path);
        payment_deliver_request_set_error(request, 404, "Not Found");
        return;
    }
}

static void payment_deliver_request_on_query_string(ebb_request * ebb_request, const char *at, size_t length) {
    payment_deliver_request_t request = ebb_request->data;
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;

    if (request->m_state != payment_deliver_request_init) return;

    if (svr->m_debug) {
        char buf[length + 1];
        memcpy(buf, at, length);
        buf[length] = 0;

        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on query_string: %s", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, buf);
    }

    if (payment_deliver_request_alloc(&blk, svr, request, length) != 0) return;
    assert(blk);

    memcpy(blk + 1, at, length);
    ringbuffer_shrink(svr->m_ringbuf, blk, length);
    payment_deliver_request_link_node_r(request, blk);
}

static void payment_deliver_request_on_fragment(ebb_request * ebb_request, const char *at, size_t length) {
    payment_deliver_request_t request = ebb_request->data;
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;

    if (svr->m_debug) {
        char buf[length + 1];
        memcpy(buf, at, length);
        buf[length] = 0;

        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on fragment: %s", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, buf);
    }
}

static void payment_deliver_request_on_body(ebb_request * ebb_request, const char *at, size_t length) {
    payment_deliver_request_t request = ebb_request->data;
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;

    if (request->m_state != payment_deliver_request_init) return;

    if (svr->m_debug) {
        char buf[length + 1];
        memcpy(buf, at, length);
        buf[length] = 0;

        CPE_INFO(
            svr->m_em, "%s: request %d at connection %d: on body: %s", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, buf);
    }

    if (payment_deliver_request_alloc(&blk, svr, request, length) != 0) return;
    assert(blk);

    memcpy(blk + 1, at, length);
    ringbuffer_shrink(svr->m_ringbuf, blk, length);
    payment_deliver_request_link_node_r(request, blk);
}

static void payment_deliver_request_on_complete(ebb_request * ebb_request) {
    payment_deliver_request_t request = ebb_request->data;
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;

    if (request->m_state != payment_deliver_request_init) return;

    if (request->m_adapter == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: on complete: no adapter!", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        payment_deliver_request_set_error(request, 404, "Not Found");
        return;
    }

    if (request->m_adapter->m_type->m_on_request(svr, request) != 0) {
        assert(request->m_state == payment_deliver_request_complete);
        return;
    }
}

void payment_deliver_request_link_node_r(payment_deliver_request_t request, ringbuffer_block_t blk) {
    if (request->m_req_blk) {
		ringbuffer_link(request->m_connection->m_svr->m_ringbuf, request->m_req_blk , blk);
	}
    else {
		blk->id = request->m_id;
		request->m_req_blk = blk;
	}
}

void payment_deliver_request_link_node_w(payment_deliver_request_t request, ringbuffer_block_t blk) {
    if (request->m_res_blk) {
		ringbuffer_link(request->m_connection->m_svr->m_ringbuf, request->m_res_blk , blk);
	}
    else {
		blk->id = request->m_id;
		request->m_res_blk = blk;
	}
}

int payment_deliver_request_alloc(ringbuffer_block_t * result, payment_deliver_svr_t svr, payment_deliver_request_t request, size_t size) {
    payment_deliver_connection_t connection = request->m_connection;
    ringbuffer_block_t blk;

    blk = ringbuffer_alloc(svr->m_ringbuf , size);
    while (blk == NULL) {
        payment_deliver_request_t free_request;
        payment_deliver_connection_t free_connection;
        int collect_id = ringbuffer_collect(svr->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(
                svr->m_em, "%s: request %d at connection %d: alloc: not enouth capacity, len=%d!",
                payment_deliver_svr_name(svr), request->m_id, connection->m_id, (int)size);
            ebb_connection_schedule_close(&connection->m_ebb_conn);
            return -1;
        }

        free_request = payment_deliver_request_find(svr, collect_id);
        assert(free_request);
        free_connection = free_request->m_connection;
        assert(free_connection);

        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: alloc: not enouth free buff, free connection %d!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, free_connection->m_id);
        ebb_connection_schedule_close(&free_connection->m_ebb_conn);
        if (free_connection == connection) return -1;

        blk = ringbuffer_alloc(svr->m_ringbuf , size);
    }

    *result = blk;
    return 0;
}

void payment_deliver_request_set_error(payment_deliver_request_t request, uint32_t http_errno, const char * http_errmsg) {
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;
    int data_len = 128;
    void * data;

    if (payment_deliver_request_alloc(&blk, svr, request, data_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_error: alloc buf error!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    if (ringbuffer_block_data(svr->m_ringbuf, blk, 0, &data) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_error: get ringbuf data fail!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    data_len = snprintf(data, data_len, "HTTP/1.1 %d %s\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n", http_errno, http_errmsg);

    ringbuffer_shrink(svr->m_ringbuf, blk, data_len);

    payment_deliver_request_link_node_w(request, blk);

    request->m_state = payment_deliver_request_complete;

    payment_deliver_connection_check_send_response(connection);
}

void payment_deliver_request_set_http_response(payment_deliver_request_t request, const char * type, const char * response) {
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;
    int head_len = 128;
    int data_len = strlen(response);
    void * data;

    /*包头 */
    if (payment_deliver_request_alloc(&blk, svr, request, head_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: write http header: alloc buf error!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    if (ringbuffer_block_data(svr->m_ringbuf, blk, 0, &data) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: write http header: get ringbuf data fail!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }
    head_len = snprintf(data, head_len, "HTTP/1.1 200 OK\r\nContent-Type: %s;charset=utf-8\r\nContent-Length: %d\r\n\r\n", type, data_len);
    ringbuffer_shrink(svr->m_ringbuf, blk, head_len);
    payment_deliver_request_link_node_w(request, blk);
    
    /*包体 */
    if (payment_deliver_request_alloc(&blk, svr, request, data_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: write http body: alloc buf error!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    if (ringbuffer_block_data(svr->m_ringbuf, blk, 0, &data) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: write http body: get ringbuf data fail!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }
    memcpy(data, response, data_len);
    ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
    payment_deliver_request_link_node_w(request, blk);

    /*设置最后的状态 */
    request->m_state = payment_deliver_request_complete;
    payment_deliver_connection_check_send_response(connection);
}

void payment_deliver_request_set_response(payment_deliver_request_t request, const char * response) {
    payment_deliver_connection_t connection = request->m_connection;
    payment_deliver_svr_t svr = connection->m_svr;
    ringbuffer_block_t blk;
    int data_len = strlen(response);
    void * data;

    if (payment_deliver_request_alloc(&blk, svr, request, data_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_error: alloc buf error!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    if (ringbuffer_block_data(svr->m_ringbuf, blk, 0, &data) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: set_error: get ringbuf data fail!",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        ringbuffer_shrink(svr->m_ringbuf, blk, data_len);
        ebb_connection_schedule_close(&connection->m_ebb_conn);
        return;
    }

    memcpy(data, response, data_len);

    ringbuffer_shrink(svr->m_ringbuf, blk, data_len);

    payment_deliver_request_link_node_w(request, blk);

    request->m_state = payment_deliver_request_complete;

    payment_deliver_connection_check_send_response(connection);
}

int payment_deliver_request_send_pkg(payment_deliver_request_t request, dp_req_t pkg, uint8_t svr_id) {
    payment_deliver_svr_t svr = request->m_connection->m_svr;
    
    return set_svr_stub_send_req_pkg(
        svr->m_stub, set_svr_svr_info_svr_type_id(svr->m_payment_svr), svr_id,
        request->m_id, pkg, NULL, 0);
}

