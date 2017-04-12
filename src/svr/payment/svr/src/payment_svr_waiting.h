#ifndef SVR_PAYMENT_SVR_WAITING_H
#define SVR_PAYMENT_SVR_WAITING_H
#include "payment_svr.h"

struct payment_svr_waiting {
    const char * m_trade_id;
    uint32_t m_require_id;
    struct cpe_hash_entry m_hh;
    char m_trade_id_buf[64];
};

payment_svr_waiting_t payment_svr_waiting_create(payment_svr_t svr, const char * trans_id);
void payment_svr_waiting_free(payment_svr_t svr, payment_svr_waiting_t waiting);
void payment_svr_waiting_free_all(payment_svr_t svr);

payment_svr_waiting_t payment_svr_waiting_find(payment_svr_t svr, const char * trade_id);

int payment_svr_waiting_start(payment_svr_t svr, const char * trans_id, logic_require_t require);
void payment_svr_waiting_stop(payment_svr_t svr, const char * trans_id, logic_require_t require);

uint32_t payment_svr_waiting_hash(const payment_svr_waiting_t waiting);
int payment_svr_waiting_eq(const payment_svr_waiting_t l, const payment_svr_waiting_t r);

#endif
