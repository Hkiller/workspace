#ifndef SVR_PAYMENT_DELIVER_SVR_QIHOO_H
#define SVR_PAYMENT_DELIVER_SVR_QIHOO_H
#include "payment_deliver_svr.h"

struct payment_deliver_adapter_qihoo_data {
    char * m_appid;
    char * m_appkey;
    char * m_appsecret;
};

struct payment_deliver_adapter_qihoo {
    struct payment_deliver_adapter_qihoo_data m_android;
};

#endif
