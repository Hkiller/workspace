#ifndef SVR_PAYMENT_DELIVER_SVR_REQUEST_H
#define SVR_PAYMENT_DELIVER_SVR_REQUEST_H
#include "payment_deliver_svr.h"

typedef enum payment_deliver_request_state {
    payment_deliver_request_init = 1
    , payment_deliver_request_runing
    , payment_deliver_request_complete
} payment_deliver_request_state_t;

struct payment_deliver_request {
    payment_deliver_connection_t m_connection;
    uint32_t m_id;
    payment_deliver_request_state_t m_state;
    ebb_request m_ebb_request;

    ringbuffer_block_t m_req_blk;
    ringbuffer_block_t m_res_blk;

    payment_deliver_adapter_t m_adapter;

    TAILQ_ENTRY(payment_deliver_request) m_next_for_connection;
    struct cpe_hash_entry m_hh_for_svr;
};

payment_deliver_request_t
payment_deliver_request_create(payment_deliver_connection_t connection);
void payment_deliver_request_free(payment_deliver_request_t request);
void payment_deliver_request_free_all(payment_deliver_connection_t connection);
payment_deliver_request_t payment_deliver_request_find(payment_deliver_svr_t svr, uint32_t id);
uint32_t payment_deliver_request_hash(payment_deliver_request_t request);
void payment_deliver_request_set_error(payment_deliver_request_t request, uint32_t http_errno, const char * http_errmsg);
void payment_deliver_request_set_response(payment_deliver_request_t request, const char * response);
void payment_deliver_request_set_http_response(payment_deliver_request_t request, const char * type, const char * response);

void payment_deliver_request_data_clear(payment_deliver_request_t request);
char * payment_deliver_request_data(payment_deliver_request_t request);

int payment_deliver_request_eq(payment_deliver_request_t l, payment_deliver_request_t r);
int payment_deliver_request_alloc(ringbuffer_block_t * result, payment_deliver_svr_t svr, payment_deliver_request_t request, size_t size);
void payment_deliver_request_link_node_r(payment_deliver_request_t request, ringbuffer_block_t blk);
void payment_deliver_request_link_node_w(payment_deliver_request_t request, ringbuffer_block_t blk);

int payment_deliver_request_send_pkg(payment_deliver_request_t request, dp_req_t pkg, uint8_t svr_id);

#endif

