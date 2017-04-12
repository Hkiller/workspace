#include <assert.h>
#include "openssl/x509.h"
#include "openssl/evp.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/base64.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils_openssl/openssl_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_json_tree.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_deliver_svr_adapter_iapppay.h"
#include "payment_deliver_svr_adapter.h"
#include "payment_deliver_svr_adapter_type.h"
#include "payment_deliver_svr_request.h"
#include "payment_deliver_svr_connection.h"

static int payment_deliver_iapppay_data_init(payment_deliver_svr_t svr, struct payment_deliver_adapter_iapppay_data * data, cfg_t cfg);
static void payment_deliver_iapppay_data_fini(payment_deliver_svr_t svr, struct payment_deliver_adapter_iapppay_data * data);

int payment_deliver_iapppay_init(payment_deliver_adapter_t adapter, cfg_t cfg) {
    payment_deliver_svr_t svr = adapter->m_svr;
    struct payment_deliver_adapter_iapppay * iapppay = (struct payment_deliver_adapter_iapppay *)adapter->m_private;
    cfg_t platform_cfg;
    
    assert(sizeof(*iapppay) <= sizeof(adapter->m_private));

    if ((platform_cfg = cfg_find_cfg(cfg, "ios"))) {
        if (payment_deliver_iapppay_data_init(svr, &iapppay->m_ios, platform_cfg) != 0) {
            return -1;
        }
    }

    if ((platform_cfg = cfg_find_cfg(cfg, "android"))) {
        if (payment_deliver_iapppay_data_init(svr, &iapppay->m_android, platform_cfg) != 0) {
            payment_deliver_iapppay_data_fini(svr, &iapppay->m_ios);
            return -1;
        }
    }
    
    return 0;
}

void payment_deliver_iapppay_fini(payment_deliver_adapter_t adapter) {
    payment_deliver_svr_t svr = adapter->m_svr;
    struct payment_deliver_adapter_iapppay * iapppay = (struct payment_deliver_adapter_iapppay *)adapter->m_private;

    payment_deliver_iapppay_data_fini(svr, &iapppay->m_ios);
    payment_deliver_iapppay_data_fini(svr, &iapppay->m_android);
}

static EVP_PKEY * payment_deliver_iapppay_make_key(payment_deliver_svr_t svr, const char * i_key, uint8_t is_public_key) {
    size_t i_key_len = strlen(i_key);
    uint8_t * key_buf;
    int skip = 0;
    int key_len;
    uint8_t * p;
    RSA * rsa;
    EVP_PKEY * key;

    key_buf = OPENSSL_malloc(i_key_len * 2);
    if (key_buf == NULL) return NULL;
    
    if (i_key[i_key_len - 1] == '=') skip++;
    if (i_key[i_key_len - 2] == '=') skip++;
    
    key_len = EVP_DecodeBlock(key_buf, (const uint8_t *)i_key, i_key_len);
    if (key_len < 0) {
        CPE_ERROR(svr->m_em, "payment_deliver: iapppay: decode key block fail");
        OPENSSL_free(key_buf);
        return NULL;
    }
    assert(key_len >= skip);
    key_len -= skip;
    key_buf[key_len] = 0;

    //d2i_RSA_PUBKEY
    p = key_buf;
    rsa = is_public_key
        ? d2i_RSA_PUBKEY(NULL, (const uint8_t **) &p, key_len)
        : d2i_RSAPrivateKey(NULL, (const uint8_t **) &p, key_len);
    if (rsa == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver: iapppay: build key fail");
        OPENSSL_free(key_buf);
        return NULL;
    }
    
    key = EVP_PKEY_new();
    if (!EVP_PKEY_assign_RSA(key, rsa)) {
        RSA_free(rsa);
        EVP_PKEY_free(key);
        return NULL;
    }
    
    OPENSSL_free(key_buf);
    return key;
}

static int payment_deliver_iapppay_data_init(payment_deliver_svr_t svr, struct payment_deliver_adapter_iapppay_data * data, cfg_t cfg) {
    const char * app_id = cfg_get_string(cfg, "app-id", NULL);
    const char * appv_key = cfg_get_string(cfg, "appv-key", NULL);
    const char * platp_key = cfg_get_string(cfg, "platp-key", NULL);

    if (app_id == NULL || appv_key == NULL || platp_key == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver: iapppay: config error, app-id=%s, appv_key=%s, platp-key=%s", app_id, appv_key, platp_key);
        return -1;
    }

    data->m_appid = cpe_str_mem_dup(svr->m_alloc, app_id);
    if (data->m_appid == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver: iapppay: alloc for appid fail!");
        return -1;
    }

    data->m_appv_key = payment_deliver_iapppay_make_key(svr, appv_key, 0);
    if (data->m_appv_key == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver: iapppay: build appv-key fail!\n%s", appv_key);
        mem_free(svr->m_alloc, data->m_appid);
        data->m_appid = NULL;
        return -1;
    }
    
    data->m_platp_key = payment_deliver_iapppay_make_key(svr, platp_key, 1);
    if (data->m_platp_key == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver: iapppay: build platp-key fail!\n%s", platp_key);

        EVP_PKEY_free(data->m_appv_key);
        data->m_appv_key = NULL;
        
        mem_free(svr->m_alloc, data->m_appid);
        data->m_appid = NULL;

        return -1;
    }
    
    return 0;
}

static void payment_deliver_iapppay_data_fini(payment_deliver_svr_t svr, struct payment_deliver_adapter_iapppay_data * data) {
    if (data->m_appid) {
        mem_free(svr->m_alloc, data->m_appid);
        data->m_appid = NULL;
    }

    if (data->m_appv_key) {
        EVP_PKEY_free(data->m_appv_key);
        data->m_appv_key = NULL;
    }

    if (data->m_platp_key) {
        EVP_PKEY_free(data->m_platp_key);
        data->m_platp_key = NULL;
    }
}

/* static */
/* struct payment_deliver_adapter_iapppay_data * */
/* payment_deliver_adapter_iapppay_env(payment_deliver_svr_t svr, payment_deliver_adapter_t adapter, PAYMENT_RECHARGE_RECORD * record) { */
/*     struct payment_deliver_adapter_iapppay * iapppay = (struct payment_deliver_adapter_iapppay *)adapter->m_private; */
    
/*     switch(record->device_category) { */
/*     case svr_payment_device_windows: */
/*         CPE_ERROR(svr->m_em, "%s: iapppay: not support device windows", payment_deliver_name(svr)); */
/*         return NULL; */
/*     case svr_payment_device_ios: */
/*         return &iapppay->m_ios; */
/*     case svr_payment_device_android: */
/*         return &iapppay->m_android; */
/*     default: */
/*         CPE_ERROR( */
/*             svr->m_em, "%s: iapppay: unknown device category %d", */
/*             payment_deliver_name(svr), record->device_category); */
/*         return NULL; */
/*     } */
/* } */

int payment_deliver_svr_iapppay_on_request(payment_deliver_svr_t svr, payment_deliver_request_t request) {
    void * data;

    data = payment_deliver_request_data(request);
    if (data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: iapppay: on request: no request data!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
    
    return 0;
}

int payment_deliver_svr_iapppay_on_response(payment_deliver_svr_t svr, payment_deliver_request_t request, int svr_error) {
    return 0;
}

