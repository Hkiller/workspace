#ifndef SVR_PAYMENT_DELIVER_SVR_DAMAI_H
#define SVR_PAYMENT_DELIVER_SVR_DAMAI_H
#include "payment_deliver_svr.h"

struct payment_deliver_adapter_damai_data {
    char * m_appid;
    char * m_appkey;
};

struct payment_deliver_adapter_damai {
    struct payment_deliver_adapter_damai_data m_android;
};

#endif
