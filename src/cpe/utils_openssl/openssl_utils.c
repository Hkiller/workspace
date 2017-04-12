#include <assert.h>
#include "openssl/evp.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils_openssl/openssl_utils.h"

int cpe_openssl_sign(
    mem_buffer_t buffer,
    const void * input, size_t input_len,
    const EVP_MD *type, EVP_PKEY * pkey, error_monitor_t em)
{
    EVP_MD_CTX * ctx = NULL;
    uint8_t * sign_buf;
    unsigned int sign_len;

    assert(input);
    assert(type);
    assert(pkey);

    ctx = EVP_MD_CTX_create();
    if (ctx == NULL) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: create ctx fail!");
        return -1;
    }
    
    if (mem_buffer_set_size(buffer, EVP_PKEY_size(pkey)) != 0) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: set buff size to %d fail!", (int)EVP_PKEY_size(pkey));
        EVP_MD_CTX_destroy(ctx);
        return -1;
    }
    
    sign_buf = mem_buffer_make_continuous(buffer, 0);
    if (sign_buf == NULL) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: alloc output buffer(size = %d) fail!", (int)mem_buffer_size(buffer));
        EVP_MD_CTX_destroy(ctx);
        return -1;
    }
    sign_len = (unsigned int)mem_buffer_size(buffer);

    if (!EVP_SignInit(ctx, type)) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: EVP_SignInit fail!");
        EVP_MD_CTX_destroy(ctx);
        return -1;
    }
    
    if (!EVP_SignUpdate(ctx, input, input_len)) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: EVP_SignUpdate fail!");
        EVP_MD_CTX_destroy(ctx);
        return -1;
    }

    if (!EVP_SignFinal(ctx, sign_buf, &sign_len, pkey)) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: calc sign fail!");
        EVP_MD_CTX_destroy(ctx);
        return -1;
    }

    mem_buffer_set_size(buffer, sign_len);
    
    return 0;
}

int cpe_openssl_sign_with_rsa(
    mem_buffer_t o_buffer, mem_buffer_t sign_buffer,
    const void * input, size_t input_len,
    const EVP_MD *type, EVP_PKEY * pkey, error_monitor_t em)
{
    unsigned char * o_buf;
    int o_len;
    
    if (cpe_openssl_sign(sign_buffer, input, input_len, type, pkey, em) != 0) return -1;
    
    mem_buffer_set_size(o_buffer, (size_t)(EVP_PKEY_size(pkey) * 2));
    o_buf = mem_buffer_make_continuous(o_buffer, 0);
    o_len = EVP_EncodeBlock(o_buf, mem_buffer_make_continuous(sign_buffer, 0), (int)mem_buffer_size(sign_buffer));
    if (o_len < 0) {
        CPE_ERROR(em, "cpe_openssl_sign_with_rsa: encode buf fail!");
        return -1;
    }
    mem_buffer_set_size(o_buffer, (size_t)o_len);
    
    return 0;
}

uint8_t cpe_openssl_verify_with_rsa(
    const void * data, size_t data_len, const char * sign, const EVP_MD *type, EVP_PKEY* pubKey)
{
    /* EVP_MD_CTX mdCtx; */
    /* uint8_t* signSrc = (uint8_t *) OPENSSL_malloc(sign.length()); */
    /* int32_t signSrcLen = base64Decode(signSrc, (const uint8_t *)sign.c_str(), sign.length()); */
    /* if(0 > signSrcLen) */
    /* { */
    /*     printf("sign base64Decode failed\n"); */
    /*     OPENSSL_free(signSrc); */
    /*     return -1; */
    /* } */

    /* EVP_VerifyInit(&mdCtx, type); */
    /* EVP_VerifyUpdate(&mdCtx, data.c_str(), data.length()); */
    /* int32_t ret = EVP_VerifyFinal(&mdCtx, signSrc, signSrcLen, pubKey); */

    /* OPENSSL_free(signSrc); */

    return 0;
}

/* int cpe_openssl_base64_decode(void * out, const uint8_t *in, int32_t inl) */
/* { */
/*     int32_t outl(0), ret(0); */

/*     if ('=' == in[inl - 1]) */
/*     { */
/*         ret++; */
/*     } */
/*     if ('=' == in[inl - 2]) */
/*     { */
/*         ret++; */
/*     } */
/*     outl = EVP_DecodeBlock(out, in, inl); */
/*     if (0 > outl) */
/*     { */
/*         printf("EVP_DecodeBlock failed\n"); */
/*         return -1; */
/*     } */
/*     out[outl - ret] = '\0'; */
/*     return outl - ret; */
/* } */
