#ifndef SVR_PAYMENT_DELIVER_SVR_CONNECTION_H
#define SVR_PAYMENT_DELIVER_SVR_CONNECTION_H
#include "payment_deliver_svr.h"

struct payment_deliver_connection {
    payment_deliver_svr_t m_svr;
    uint32_t m_id;
    ebb_connection m_ebb_conn;
    payment_deliver_request_list_t m_requests;

    TAILQ_ENTRY(payment_deliver_connection) m_next_for_svr;
};

payment_deliver_connection_t
payment_deliver_connection_create(payment_deliver_svr_t svr);
void payment_deliver_connection_check_send_response(payment_deliver_connection_t connection);
void payment_deliver_connection_free(payment_deliver_connection_t connection);
void payment_deliver_connection_free_all(payment_deliver_svr_t svr);

#endif
