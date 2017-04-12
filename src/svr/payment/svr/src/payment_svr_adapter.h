#ifndef SVR_PAYMENT_SVR_ADAPTER_H
#define SVR_PAYMENT_SVR_ADAPTER_H
#include "payment_svr.h"

struct payment_svr_adapter {
    payment_svr_t m_svr;
    TAILQ_ENTRY(payment_svr_adapter) m_next;
    payment_svr_adapter_type_t m_type;
    char m_private[128];
};

payment_svr_adapter_t payment_svr_adapter_create(payment_svr_t svr, payment_svr_adapter_type_t type, cfg_t cfg);
void payment_svr_adapter_free(payment_svr_adapter_t adapter);

payment_svr_adapter_t payment_svr_adapter_find_by_type_id(payment_svr_t svr, uint8_t type_id);

int payment_svr_adapter_load(payment_svr_t svr, cfg_t cfg);

#endif
