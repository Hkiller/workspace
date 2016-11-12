#ifndef CPE_OPENSSL_UTILS_H
#define CPE_OPENSSL_UTILS_H
#include "openssl/ossl_typ.h"
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int cpe_openssl_sign(
    mem_buffer_t buffer,
    const void * input, size_t input_len,
    const EVP_MD *type, EVP_PKEY * pkey, error_monitor_t em);

int cpe_openssl_sign_with_rsa(
    mem_buffer_t o_buffer, mem_buffer_t sign_buffer,
    const void * input, size_t input_len,
    const EVP_MD *type, EVP_PKEY * pkey, error_monitor_t em);

uint8_t cpe_openssl_verify_with_rsa(
    const void * data, size_t data_len, const char * sign, const EVP_MD * type, EVP_PKEY * pkey);
    
#ifdef __cplusplus
}
#endif

#endif
