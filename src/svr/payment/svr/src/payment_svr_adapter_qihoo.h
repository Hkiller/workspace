#ifndef SVR_PAYMENT_SVR_QIHOO_H
#define SVR_PAYMENT_SVR_QIHOO_H
#include "gd/net_trans/net_trans_types.h"
#include "payment_svr.h"

struct payment_svr_adapter_qihoo_data {
    char * m_appid;
    char * m_appkey;
    char * m_appsecret;
};

struct payment_svr_adapter_qihoo {
    net_trans_group_t m_trans_group;
    uint32_t m_timeout_s;
    struct payment_svr_adapter_qihoo_data m_android;
};

#endif
