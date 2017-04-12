#include "cpe/pal/pal_strings.h"
#include "cpe/utils/md5.h"
#include "gd/app/app_context.h"
#include "mongo_manip_i.h"

mongo_pkg_t mongo_pkg_build_check_is_master(mongo_driver_t driver) {
    mongo_pkg_t pkg_buf;

    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build check-is-master: get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "ismaster", 1) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build check-is-master: build pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}

mongo_pkg_t mongo_pkg_build_check_readable(mongo_driver_t driver) {
    mongo_pkg_t pkg_buf;

    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build check-sliver-ok: get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    mongo_pkg_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    mongo_pkg_set_op(pkg_buf, mongo_db_op_query);
    mongo_pkg_set_collection(pkg_buf, "system.version");

    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build check-sliver-ok: build pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}

mongo_pkg_t mongo_pkg_build_cr_getnonce(mongo_driver_t driver) {
    mongo_pkg_t pkg_buf;

    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build getnonce: get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "getnonce", 1) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build getnonce: build pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}

const char * mongo_pkg_build_authenticate_pass_digest(mongo_driver_t driver) {
    struct cpe_md5_ctx md5_ctx;

    cpe_md5_ctx_init(&md5_ctx);
    cpe_md5_ctx_update(&md5_ctx, driver->m_user, strlen(driver->m_user));
    cpe_md5_ctx_update(&md5_ctx, ":mongo:", 7);
    cpe_md5_ctx_update(&md5_ctx, driver->m_passwd, strlen(driver->m_passwd));
    cpe_md5_ctx_final(&md5_ctx);

    return cpe_md5_dump(&md5_ctx.value, gd_app_tmp_buffer(driver->m_app));
}

mongo_pkg_t mongo_pkg_build_cr_authenticate(mongo_driver_t driver, const char * nonce) {
    mongo_pkg_t pkg_buf;
    struct cpe_md5_ctx md5_ctx;
    const char * pass_digest;
    
    pkg_buf = mongo_driver_pkg_buf(driver);
    if (pkg_buf == NULL) {
        CPE_ERROR(driver->m_em, "%s: build authenticate(cr): get pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    pass_digest = mongo_pkg_build_authenticate_pass_digest(driver);
    
    cpe_md5_ctx_init(&md5_ctx);
    cpe_md5_ctx_update(&md5_ctx, nonce, strlen(nonce));
    cpe_md5_ctx_update(&md5_ctx, driver->m_user, strlen(driver->m_user));
    cpe_md5_ctx_update(&md5_ctx, pass_digest, strlen(pass_digest));
    cpe_md5_ctx_final(&md5_ctx);
    
    mongo_pkg_cmd_init(pkg_buf);
    mongo_pkg_set_db(pkg_buf, driver->m_source);
    if (mongo_pkg_doc_open(pkg_buf) != 0
        || mongo_pkg_append_int32(pkg_buf, "authenticate", 1) != 0
        || mongo_pkg_append_string(pkg_buf, "user", driver->m_user) != 0
        || mongo_pkg_append_string(pkg_buf, "nonce", nonce) != 0
        || mongo_pkg_append_string(pkg_buf, "key", cpe_md5_dump(&md5_ctx.value, gd_app_tmp_buffer(driver->m_app))) != 0
        || mongo_pkg_doc_close(pkg_buf) != 0)
    {
        CPE_ERROR(driver->m_em, "%s: build authenticate(cr): build pkg buf fail!", mongo_driver_name(driver));
        return NULL;
    }

    return pkg_buf;
}
