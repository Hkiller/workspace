#ifndef SVR_PAYMENT_DELIVER_SVR_IAPPPAY_H
#define SVR_PAYMENT_DELIVER_SVR_IAPPPAY_H
#include "openssl/ossl_typ.h"
#include "payment_deliver_svr.h"

struct payment_deliver_adapter_iapppay_data {
    char * m_appid;
    EVP_PKEY * m_appv_key;
    EVP_PKEY * m_platp_key;
};

struct payment_deliver_adapter_iapppay {
    struct payment_deliver_adapter_iapppay_data m_ios;
    struct payment_deliver_adapter_iapppay_data m_android;
};

#endif
