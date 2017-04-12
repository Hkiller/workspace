#ifndef SVR_PAYMENT_SVR_IAPPPAY_H
#define SVR_PAYMENT_SVR_IAPPPAY_H
#include "openssl/ossl_typ.h"
#include "gd/net_trans/net_trans_types.h"
#include "payment_svr.h"

struct payment_svr_adapter_iapppay_data {
    char * m_appid;
    EVP_PKEY * m_appv_key;
    EVP_PKEY * m_platp_key;
};

struct payment_svr_adapter_iapppay {
    net_trans_group_t m_trans_group;
    struct payment_svr_adapter_iapppay_data m_ios;
    struct payment_svr_adapter_iapppay_data m_android;
};

#endif
