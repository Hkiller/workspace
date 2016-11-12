#include "openssl/sha.h"
#include "openssl/evp.h"
#include "openssl/hmac.h"
#include "openssl/rand.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/md5.h"
#include "cpe/utils/base64.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "mongo_manip_i.h"
#include "mongo_connection_i.h"

#define MONGO_SCRAM_SERVER_KEY "Server Key"
#define MONGO_SCRAM_CLIENT_KEY "Client Key"

#define MONGO_SCRAM_HASH_SIZE 20

#define MONGO_SCRAM_B64_ENCODED_SIZE(n) (2 * n)

#define MONGO_SCRAM_B64_HASH_SIZE \
   MONGO_SCRAM_B64_ENCODED_SIZE (MONGO_SCRAM_HASH_SIZE)

struct scram_ctx {
    uint8_t m_salted_password[MONGO_SCRAM_HASH_SIZE];
    char m_encoded_nonce[48];
    size_t m_encoded_nonce_len;
    struct mem_buffer m_auth_message;
};

typedef struct scram_ctx * scram_ctx_t;

static void mongo_pkg_build_scram_password(
    scram_ctx_t scram, const char * password, uint32_t password_len, const uint8_t  *salt, uint32_t salt_len, uint32_t iterations);
static uint8_t mongo_pkg_build_scram_generate_client_proof(scram_ctx_t scram, mem_buffer_t buffer);
static uint8_t mongo_pkg_build_scram_verify_server_signature(scram_ctx_t scram,char * verification, uint32_t len);

static void scram_ctx_free(mongo_driver_t driver, void * p) {
    scram_ctx_t scram = p;
    mem_buffer_clear(&scram->m_auth_message);
    mem_free(driver->m_alloc, scram);
}

static scram_ctx_t scram_ctx_create(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    scram_ctx_t scram;
    uint8_t nonce[24];
    struct write_stream_mem ws;
    struct read_stream_mem rs;
    
    scram = mem_alloc(driver->m_alloc, sizeof(struct scram_ctx));
    if (scram == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): alloc scram ctx fail!", mongo_driver_name(driver));
        return NULL;
    }

    /*nonce*/
    if (RAND_bytes(nonce, sizeof(nonce)) != 1) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): rand bytes fail!", mongo_driver_name(driver));
        mem_free(driver->m_alloc, scram);
        return NULL;
    }

    write_stream_mem_init(&ws, scram->m_encoded_nonce, sizeof(scram->m_encoded_nonce));
    read_stream_mem_init(&rs, nonce, sizeof(nonce));
    scram->m_encoded_nonce_len = cpe_base64_encode((write_stream_t)&ws, (read_stream_t)&rs);
    assert(scram->m_encoded_nonce_len < sizeof(scram->m_encoded_nonce));
    scram->m_encoded_nonce[scram->m_encoded_nonce_len] = 0;
    
    mem_buffer_init(&scram->m_auth_message, driver->m_alloc);
    
    assert(connection->m_addition == NULL);
    connection->m_addition = scram;
    connection->m_addition_cleanup = scram_ctx_free;

    return scram;
}

mongo_pkg_t mongo_pkg_build_scram_start(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    mongo_pkg_t pkg_buf;
    mem_buffer_t msg_buf;
    scram_ctx_t scram;
    const char * ptr;
    
    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): start: get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    scram = scram_ctx_create(connection);
    if (scram == NULL) return NULL;
    
    /**/
    /*构建消息 */
    msg_buf = gd_app_tmp_buffer(driver->m_app);
    mem_buffer_clear_data(msg_buf);

    mem_buffer_append(msg_buf, "n,,n=", 5);
    for (ptr = driver->m_user; *ptr; ptr++) {
        /* RFC 5802 specifies that ',' and '=' and encoded as '=2C' and '=3D'
         * respectively in the user name */
        switch (*ptr) {
        case ',':
            mem_buffer_append(msg_buf, "=2C", 3);
            break;
        case '=':
            mem_buffer_append(msg_buf, "=3D", 3);
            break;
        default:
            mem_buffer_append_char(msg_buf, *ptr);
            break;
        }
    }
    mem_buffer_append(msg_buf, ",r=", 3);
    mem_buffer_append(msg_buf, scram->m_encoded_nonce, scram->m_encoded_nonce_len);

    /* we have to keep track of the conversation to create a client proof later
     * on.  This copies the message we're crafting from the 'n=' portion onwards
     * into a buffer we're managing */
    mem_buffer_append(&scram->m_auth_message, mem_buffer_make_continuous(msg_buf, 0) + 3, mem_buffer_size(msg_buf) - 3);
    mem_buffer_append_char(&scram->m_auth_message, ',');
    
    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "saslStart", 1) != 0
        || mongo_pkg_append_string(pkg_buf, "mechanism", "SCRAM-SHA-1") != 0
        || mongo_pkg_append_binary(pkg_buf, "payload", BSON_SUBTYPE_BINARY, mem_buffer_make_continuous(msg_buf, 0), mem_buffer_size(msg_buf)) != 0
        || mongo_pkg_append_int32(pkg_buf, "autoAuthorize", 1) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): start: build pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}

mongo_pkg_t mongo_pkg_build_scram_step2(mongo_connection_t connection, int32_t conv_id, char * payload) {
    mongo_driver_t driver = connection->m_server->m_driver;
    size_t payload_len = strlen(payload);
    struct write_stream_mem ws;
    struct read_stream_mem rs;
    mongo_pkg_t pkg_buf;
    mem_buffer_t msg_buf;
    scram_ctx_t scram;
    char * val_r;
    char * val_s;
    char * val_i;
    char hashed_passwd[64];

    uint8_t decoded_salt[MONGO_SCRAM_B64_HASH_SIZE];
    int32_t decoded_salt_len;

    char *tmp;
    int iterations;

    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step2: get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    scram = connection->m_addition;
    assert(scram);

    /*passwd*/
    cpe_str_dup(hashed_passwd, sizeof(hashed_passwd), mongo_pkg_build_authenticate_pass_digest(driver));
    
    /* we need all of the incoming message for the final client proof */
    mem_buffer_append(&scram->m_auth_message, payload, payload_len);
    mem_buffer_append_char(&scram->m_auth_message, ',');

    if ((val_r = cpe_str_read_and_remove_arg(payload, "r", ',', '=')) == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step2: arg r not exist!", mongo_driver_name(driver));
        return NULL;
    }

    if ((val_s = cpe_str_read_and_remove_arg(payload, "s", ',', '=')) == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step2: arg s not exist!", mongo_driver_name(driver));
        return NULL;
    }
    
    if ((val_i = cpe_str_read_and_remove_arg(payload, "i", ',', '=')) == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step2: arg i not exist!", mongo_driver_name(driver));
        return NULL;
    }

    /* verify our nonce */
    if (!cpe_str_start_with(val_r, scram->m_encoded_nonce)) {
        CPE_ERROR(
            driver->m_em, "%s: build authenticate(scram): step2: nonce verify fail, %s ==> %s!",
            mongo_driver_name(driver), scram->m_encoded_nonce, val_r);
        return NULL;
    }

    /*构建消息 */
    msg_buf = gd_app_tmp_buffer(driver->m_app);
    mem_buffer_clear_data(msg_buf);

    mem_buffer_append(msg_buf, "c=biws,r=", 9);
    mem_buffer_append(msg_buf, val_r, strlen(val_r));

    mem_buffer_append(&scram->m_auth_message, mem_buffer_make_continuous(msg_buf, 0), mem_buffer_size(msg_buf));

    mem_buffer_append(msg_buf, ",p=", 3);

    write_stream_mem_init(&ws, decoded_salt, sizeof(decoded_salt));
    read_stream_mem_init(&rs, val_s, strlen(val_s));
    decoded_salt_len = cpe_base64_decode((write_stream_t)&ws, (read_stream_t)&rs);
    if (decoded_salt_len != 16) {
        CPE_ERROR(
            driver->m_em, "%s: build authenticate(scram): step2: salt len fail, expect 16, but %d!",
            mongo_driver_name(driver), decoded_salt_len);
        return NULL;
    }

    iterations = (int) strtoll(val_i, &tmp, 10);
    if (*tmp) {
        CPE_ERROR(
            driver->m_em, "%s: build authenticate(scram): step2: SCRAM Failure: unable to parse iterations",
            mongo_driver_name(driver));
        return NULL;
    }

    mongo_pkg_build_scram_password(
        scram, hashed_passwd, (uint32_t)strlen(hashed_passwd), decoded_salt, decoded_salt_len, iterations);

    mongo_pkg_build_scram_generate_client_proof(scram, msg_buf);

    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "saslContinue", 1) != 0
        || mongo_pkg_append_int32(pkg_buf, "conversationId", 1) != 0
        || mongo_pkg_append_binary(pkg_buf, "payload", BSON_SUBTYPE_BINARY, mem_buffer_make_continuous(msg_buf, 0), mem_buffer_size(msg_buf)) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step2: build pkg fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}

mongo_pkg_t mongo_pkg_build_scram_step3(mongo_connection_t connection, int32_t conv_id, char * payload) {
    mongo_driver_t driver = connection->m_server->m_driver;
    mongo_pkg_t pkg_buf;
    scram_ctx_t scram;
    char * val_e;
    char * val_v;

    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step3: get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    scram = connection->m_addition;
    assert(scram);

    if ((val_e = cpe_str_read_and_remove_arg(payload, "e", ',', '='))) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step3: SCRAM Failure %s!", mongo_driver_name(driver), val_e);
        return NULL;
    }

    if ((val_v = cpe_str_read_and_remove_arg(payload, "v", ',', '=')) == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step3: arg v not exist!", mongo_driver_name(driver));
        return NULL;
    }

    if (!mongo_pkg_build_scram_verify_server_signature(scram, val_v, strlen(val_v))) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): step3: verify server signature fail!", mongo_driver_name(driver));
        return NULL;
    }
    
    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "saslContinue", 1) != 0
        || mongo_pkg_append_int32(pkg_buf, "conversationId", 1) != 0
        || mongo_pkg_append_binary(pkg_buf, "payload", BSON_SUBTYPE_BINARY, NULL, 0) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build authenticate(scram): build step3 pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}

/* Compute the SCRAM step Hi() as defined in RFC5802 */
static void mongo_pkg_build_scram_password(
    scram_ctx_t scram, const char * password, uint32_t password_len, const uint8_t  *salt, uint32_t salt_len, uint32_t iterations)
{
   uint8_t intermediate_digest[MONGO_SCRAM_HASH_SIZE];
   uint8_t start_key[MONGO_SCRAM_HASH_SIZE];
   int i;
   int k;
   uint8_t *output = scram->m_salted_password;

   memcpy(start_key, salt, salt_len);

   start_key[salt_len] = 0;
   start_key[salt_len + 1] = 0;
   start_key[salt_len + 2] = 0;
   start_key[salt_len + 3] = 1;

   /* U1 = HMAC(input, salt + 0001) */
   HMAC(EVP_sha1(), password, password_len, start_key, sizeof(start_key), output, NULL);

   memcpy (intermediate_digest, output, MONGO_SCRAM_HASH_SIZE);

   /* intermediateDigest contains Ui and output contains the accumulated XOR:ed result */
   for (i = 2; i <= iterations; i++) {
       HMAC(EVP_sha1(), password, password_len, intermediate_digest, sizeof(intermediate_digest), intermediate_digest, NULL);
       for (k = 0; k < MONGO_SCRAM_HASH_SIZE; k++) {
           output[k] ^= intermediate_digest[k];
       }
   }
}

#if OPENSSL_VERSION_NUMBER < 0x10100000L
EVP_MD_CTX *EVP_MD_CTX_new(void)
{
    return bson_malloc0 (sizeof (EVP_MD_CTX));
}

void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
    EVP_MD_CTX_cleanup (ctx);
    bson_free (ctx);
}
#endif

static uint8_t mongo_pkg_build_scram_sha1(const unsigned char *input, const size_t input_len, unsigned char *output) {
   EVP_MD_CTX *digest_ctxp = EVP_MD_CTX_new();
   uint8_t rval = 0;

   if (1 != EVP_DigestInit_ex (digest_ctxp, EVP_sha1 (), NULL)) {
	   goto cleanup;
   }

   if (1 != EVP_DigestUpdate (digest_ctxp, input, input_len)) {
	   goto cleanup;
   }

   rval = (1 == EVP_DigestFinal_ex (digest_ctxp, output, NULL)) ? 1 : 0;

cleanup:
   EVP_MD_CTX_free (digest_ctxp);

   return rval;
}

static uint8_t mongo_pkg_build_scram_generate_client_proof(scram_ctx_t scram, mem_buffer_t buffer) {
    uint8_t client_key[MONGO_SCRAM_HASH_SIZE];
    uint8_t stored_key[MONGO_SCRAM_HASH_SIZE];
    uint8_t client_signature[MONGO_SCRAM_HASH_SIZE];
    unsigned char client_proof[MONGO_SCRAM_HASH_SIZE];
    struct write_stream_buffer ws;
    struct read_stream_mem rs;
    int i;

    /* ClientKey := HMAC(saltedPassword, "Client Key") */
    HMAC(EVP_sha1(), scram->m_salted_password, MONGO_SCRAM_HASH_SIZE,
         (uint8_t *)MONGO_SCRAM_CLIENT_KEY, strlen(MONGO_SCRAM_CLIENT_KEY), client_key, NULL);

    /* StoredKey := H(client_key) */
    mongo_pkg_build_scram_sha1(client_key, MONGO_SCRAM_HASH_SIZE, stored_key);

    /* ClientSignature := HMAC(StoredKey, AuthMessage) */
    HMAC(EVP_sha1(), stored_key, MONGO_SCRAM_HASH_SIZE,
         mem_buffer_make_continuous(&scram->m_auth_message, 0), mem_buffer_size(&scram->m_auth_message),
         client_signature, NULL);

    /* ClientProof := ClientKey XOR ClientSignature */

    for (i = 0; i < MONGO_SCRAM_HASH_SIZE; i++) {
        client_proof[i] = client_key[i] ^ client_signature[i];
    }

    write_stream_buffer_init(&ws, buffer);
    read_stream_mem_init(&rs, client_proof, sizeof(client_proof));
    cpe_base64_encode((write_stream_t)&ws, (read_stream_t)&rs);

    return 1;
}

static uint8_t mongo_pkg_build_scram_verify_server_signature(scram_ctx_t scram, char * verification, uint32_t len) {
    uint8_t server_key[MONGO_SCRAM_HASH_SIZE];
    char encoded_server_signature[MONGO_SCRAM_B64_HASH_SIZE];
    int32_t encoded_server_signature_len;
    uint8_t server_signature[MONGO_SCRAM_HASH_SIZE];
    struct write_stream_mem ws;
    struct read_stream_mem rs;

    /* ServerKey := HMAC(SaltedPassword, "Server Key") */
    HMAC(EVP_sha1(), scram->m_salted_password, MONGO_SCRAM_HASH_SIZE,
         (uint8_t *)MONGO_SCRAM_SERVER_KEY, strlen(MONGO_SCRAM_SERVER_KEY), server_key, NULL);

    /* ServerSignature := HMAC(ServerKey, AuthMessage) */
    HMAC(EVP_sha1(), server_key, MONGO_SCRAM_HASH_SIZE,
         (uint8_t *)mem_buffer_make_continuous(&scram->m_auth_message, 0), mem_buffer_size(&scram->m_auth_message),
         server_signature, NULL);

    write_stream_mem_init(&ws, encoded_server_signature, sizeof(encoded_server_signature));
    read_stream_mem_init(&rs, server_signature, sizeof (server_signature));
    encoded_server_signature_len = cpe_base64_encode((write_stream_t)&ws, (read_stream_t)&rs);

    return (len == encoded_server_signature_len && memcmp(verification, encoded_server_signature, len) == 0) ? 1 : 0;
}

