#ifndef SVR_PAYMENT_DELIVER_SVR_ADAPTER_H
#define SVR_PAYMENT_DELIVER_SVR_ADAPTER_H
#include "payment_deliver_svr.h"

struct payment_deliver_adapter {
    payment_deliver_svr_t m_svr;
    TAILQ_ENTRY(payment_deliver_adapter) m_next;
    payment_deliver_adapter_type_t m_type;
    char m_private[128];
};

payment_deliver_adapter_t payment_deliver_adapter_create(payment_deliver_svr_t svr, payment_deliver_adapter_type_t type, cfg_t cfg);
void payment_deliver_adapter_free(payment_deliver_adapter_t adapter);

payment_deliver_adapter_t payment_deliver_adapter_find_by_name(payment_deliver_svr_t svr, const char * path);

int payment_deliver_adapter_load(payment_deliver_svr_t svr, cfg_t cfg);

#endif
