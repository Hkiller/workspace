#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_bson.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "protocol/mongo_cli/mongo_cli.h"
#include "mongo_cli_internal_ops.h"

static mongo_cli_pkg_parse_result_t
mongo_cli_proxy_recv_build_results(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, logic_require_t require, logic_data_t * result_data,
    LPDRMETA result_meta, const char * result_prefix, struct mongo_cli_require_keep * keep_data)
{
    struct mongo_doc_it doc_it;
    mongo_doc_t doc;

    mongo_pkg_doc_it(&doc_it, pkg);
    while((doc = mongo_pkg_doc_it_next(&doc_it))) {
        if (result_prefix) {
            doc = mongo_doc_find_doc(doc, result_prefix);
            if (doc == NULL) {
                CPE_ERROR(proxy->m_em, "%s: recv_response: find doc at %s fail!", mongo_cli_proxy_name(proxy), result_prefix);
                return mongo_cli_pkg_parse_fail;
            }
        }

        if (keep_data->m_parser) {
            mongo_cli_pkg_parse_result_t parse_result = 
                keep_data->m_parser(
                    keep_data->m_parse_ctx, require, pkg, result_data, result_meta,
                    mongo_cli_pkg_parse_data, mongo_doc_data(doc), mongo_doc_size(doc));
            if (parse_result != mongo_cli_pkg_parse_next) return parse_result;
        }
        else {
            void * result = logic_data_record_append_auto_inc(result_data);
            if (result == NULL) {
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: append record fail, record count is %d!",
                    mongo_cli_proxy_name(proxy), (int)logic_data_record_count(*result_data));
                return mongo_cli_pkg_parse_fail;
            }

            if (dr_bson_read(result, dr_meta_size(result_meta), mongo_doc_data(doc), mongo_doc_size(doc), result_meta, proxy->m_em) < 0) {
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: bson read fail, meta=%s, input-len=%d!",
                    mongo_cli_proxy_name(proxy), dr_meta_name(result_meta), (int)mongo_doc_size(doc));
                return mongo_cli_pkg_parse_fail;
            }
        }
    }

    return mongo_cli_pkg_parse_next;
}

static mongo_cli_pkg_parse_result_t
mongo_cli_proxy_recv_build_result_from_it(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, bson_iter_t * bson_it, logic_require_t require, logic_data_t * result_data, LPDRMETA result_meta,
    const char * result_prefix, struct mongo_cli_require_keep * keep_data)
{
    void * result;
    bson_type_t type = bson_iter_type(bson_it);
    uint32_t data_len;
    const uint8_t * data_buf;

    if (type != BSON_TYPE_DOCUMENT) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: find_and_modify: not support bson type %d!", mongo_cli_proxy_name(proxy), type);
        return mongo_cli_pkg_parse_fail;
    }

    if (result_prefix) {
        if (!bson_iter_find(bson_it, result_prefix)) {
            CPE_ERROR(proxy->m_em, "%s: recv_response: find sub object at %s fail!", mongo_cli_proxy_name(proxy), result_prefix);
            return mongo_cli_pkg_parse_fail;
        }
        else if (bson_iter_type(bson_it) != BSON_TYPE_DOCUMENT) {
            CPE_ERROR(
                proxy->m_em, "%s: recv_response: find sub object at %s is not object, type=%d!",
                mongo_cli_proxy_name(proxy), result_prefix, type);
            return mongo_cli_pkg_parse_fail;
        }
    }

    bson_iter_document(bson_it, &data_len, &data_buf);

    if (keep_data->m_parser) {
        mongo_cli_pkg_parse_result_t parse_result = 
            keep_data->m_parser(
                keep_data->m_parse_ctx, require, pkg, result_data, result_meta,
                mongo_cli_pkg_parse_data, data_buf, data_len);
        switch(parse_result) {
        case mongo_cli_pkg_parse_success:
            return parse_result;
        case mongo_cli_pkg_parse_fail:
            if (proxy->m_debug) {
                CPE_INFO(proxy->m_em, "%s: recv_response: user parse fail!", mongo_cli_proxy_name(proxy));
            }
            return parse_result;
        case mongo_cli_pkg_parse_next:
            return mongo_cli_pkg_parse_next;
        default:
            CPE_ERROR(proxy->m_em, "%s: recv_response: user parse result %d unknown!", mongo_cli_proxy_name(proxy), parse_result);
            return mongo_cli_pkg_parse_fail;
        }
    }
    else {
        result = logic_data_record_append_auto_inc(result_data);
        if (result == NULL) {
            CPE_ERROR(
                proxy->m_em, "%s: recv_response: append record fail, record count is %d!",
                mongo_cli_proxy_name(proxy), (int)logic_data_record_count(*result_data));
            return mongo_cli_pkg_parse_fail;
        }

        if (dr_bson_read(result, dr_meta_size(result_meta), data_buf, data_len, result_meta, proxy->m_em) < 0) {
            CPE_ERROR(proxy->m_em, "%s: recv_response: bson read fail, len=%d!", mongo_cli_proxy_name(proxy), data_len);
            return mongo_cli_pkg_parse_fail;
        }

        return mongo_cli_pkg_parse_next;
    }
}

static mongo_cli_pkg_parse_result_t
mongo_cli_proxy_recv_process_find_and_modify(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, logic_require_t require, logic_data_t * result_data, LPDRMETA result_meta,
    const char * result_prefix, struct mongo_cli_require_keep * keep_data)
{
    bson_iter_t bson_it;
    logic_data_t last_error_data;
    MONGO_LASTERROR * last_error;

    if (mongo_pkg_find(&bson_it, pkg, 0, "ok") != 0) {
        if (mongo_pkg_find(&bson_it, pkg, 0, "code") == 0) {
            int32_t err = bson_iter_int32(&bson_it);
            if (proxy->m_debug) {
                CPE_INFO(proxy->m_em, "%s: recv_response: find_and_modify: error: %d!", mongo_cli_proxy_name(proxy), err);
            }
            return mongo_cli_pkg_parse_fail;
        }
        else {
            if (proxy->m_debug) {
                CPE_INFO(proxy->m_em, "%s: recv_response: find_and_modify: error: (no error code)!", mongo_cli_proxy_name(proxy));
            }
            return mongo_cli_pkg_parse_fail;
        }
    }

    /*读取数据 */
    if (mongo_pkg_find(&bson_it, pkg, 0, "value") != 0) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: find_and_modify: find value fail!", mongo_cli_proxy_name(proxy));
        return mongo_cli_pkg_parse_fail;
    }

    switch(bson_iter_type(&bson_it)) {
    case BSON_TYPE_NULL:
        break;
    case BSON_TYPE_DOCUMENT: {
        mongo_cli_pkg_parse_result_t parse_result;

        parse_result =
            mongo_cli_proxy_recv_build_result_from_it(proxy, pkg, &bson_it, require, result_data, result_meta, result_prefix, keep_data);
        if (parse_result != mongo_cli_pkg_parse_next) return parse_result;

        break;
    }
    case BSON_TYPE_ARRAY: {
        uint32_t array_len;
        const uint8_t * array_buf;
        bson_t child_data;
        
        bson_iter_array(&bson_it, &array_len, &array_buf);

        if (bson_init_static(&child_data, array_buf, array_len)) {
            bson_iter_t child_it;
            if (bson_iter_init(&child_it, &child_data)) {
                while(bson_iter_next(&child_it)) {
                    mongo_cli_pkg_parse_result_t parse_result;

                    parse_result =
                        mongo_cli_proxy_recv_build_result_from_it(proxy, pkg, &child_it, require, result_data, result_meta, result_prefix, keep_data);
                    if (parse_result != mongo_cli_pkg_parse_next) return parse_result;
                }
            }
        }
        
        break;
    }
    default:
        CPE_ERROR(
            proxy->m_em, "%s: recv_response: find_and_modify: value type %d error!", 
            mongo_cli_proxy_name(proxy), bson_iter_type(&bson_it));
        return mongo_cli_pkg_parse_fail;
    }

    /*构造LAST_ERROR */
    last_error_data = logic_require_data_get_or_create(require, proxy->m_meta_lasterror, sizeof(MONGO_LASTERROR));
    if (last_error_data == NULL) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: find_and_modify: create lastErrorObject data fail!", mongo_cli_proxy_name(proxy));
        return mongo_cli_pkg_parse_fail;
    }
    last_error = logic_data_data(last_error_data);
    bzero(last_error, sizeof(MONGO_LASTERROR));

    if (mongo_pkg_find(&bson_it, pkg, 0, "lastErrorObject") == 0) {
        uint32_t data_len;
        const uint8_t * data_buf;
        bson_iter_document(&bson_it, &data_len, &data_buf);
        if (dr_bson_read(last_error, sizeof(MONGO_LASTERROR), data_buf, data_len, proxy->m_meta_lasterror, proxy->m_em) < 0) {
            CPE_ERROR(proxy->m_em, "%s: recv_response: bson read lastErrorObject fail!", mongo_cli_proxy_name(proxy));
            return mongo_cli_pkg_parse_fail;
        }
    }

    return mongo_cli_pkg_parse_next;
}

static mongo_cli_pkg_parse_result_t
mongo_cli_proxy_recv_process_find_last_error(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, logic_require_t require, logic_data_t result_data)
{
    MONGO_LASTERROR * lasterror;
    mongo_doc_t doc;

    if (logic_data_record_count(result_data) != 1) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: getlasterror: result count error!", mongo_cli_proxy_name(proxy));
        return mongo_cli_pkg_parse_fail;
    }

    if (mongo_pkg_doc_count(pkg) != 1) {
        CPE_ERROR(
            proxy->m_em, "%s: recv_response: getlasterror: result pkg doc count %d error!",
            mongo_cli_proxy_name(proxy), (int)mongo_pkg_doc_count(pkg));
        return mongo_cli_pkg_parse_fail;
    }

    doc = mongo_pkg_doc_at(pkg, 0);
    if (doc == NULL) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: getlasterror: get doc error!", mongo_cli_proxy_name(proxy));
        return mongo_cli_pkg_parse_fail;
    }

    lasterror = (MONGO_LASTERROR *)logic_data_record_at(result_data, 0);

    if (dr_bson_read(lasterror, sizeof(*lasterror), mongo_doc_data(doc), mongo_doc_size(doc), proxy->m_meta_lasterror, proxy->m_em) < 0) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: getlasterror: bson read fail!", mongo_cli_proxy_name(proxy));
        return mongo_cli_pkg_parse_fail;
    }

    if (lasterror->code == 0) {
        if (proxy->m_debug) {
            CPE_INFO(proxy->m_em, "%s: recv_response: req %d: ok", mongo_cli_proxy_name(proxy), logic_require_id(require));
        }

        logic_require_set_done(require);
        return mongo_cli_pkg_parse_next;
    }
    else {
        if (proxy->m_debug) {
            CPE_INFO(
                proxy->m_em, "%s: recv_response: req %d: error: %d %s",
                mongo_cli_proxy_name(proxy), logic_require_id(require), lasterror->code, lasterror->err);
        }

        logic_require_set_error_ex(require, lasterror->code);
        return mongo_cli_pkg_parse_fail;
    }
}

int mongo_cli_proxy_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    struct mongo_cli_proxy * proxy;
    mongo_pkg_t pkg;
    uint32_t sn;
    logic_require_t require;
    struct mongo_cli_require_keep keep_data;
    size_t keep_data_len = sizeof(keep_data);
    logic_data_t result_data = NULL;
    mongo_cli_pkg_parse_result_t parse_result = mongo_cli_pkg_parse_next;

    proxy = (struct mongo_cli_proxy *)ctx;

    pkg = mongo_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(em, "%s: cast to pkg fail!", mongo_cli_proxy_name(proxy));
        return -1;
    }

    sn = mongo_pkg_response_to(pkg);
    require = logic_require_queue_remove_get(proxy->m_require_queue, sn, &keep_data, &keep_data_len);
    if (require == NULL) {
        CPE_ERROR(em, "%s: require %d not exist in queue!", mongo_cli_proxy_name(proxy), sn);
        return -1;
    }

    if (keep_data.m_parser) {
        parse_result = keep_data.m_parser(
            keep_data.m_parse_ctx, require, pkg, &result_data, keep_data.m_result_meta,
            mongo_cli_pkg_parse_begin, NULL, 0);
        if (parse_result == mongo_cli_pkg_parse_fail) goto RECV_COMPLETE;
    }

    if (mongo_pkg_op(pkg) == mongo_db_op_replay) {
        const char * result_prefix = NULL;
        LPDRMETA result_meta = NULL;

        if (result_data == NULL && keep_data.m_result_meta) {
            ssize_t data_capacity = 
                keep_data.m_result_count_init == 1
                ? dr_meta_size(keep_data.m_result_meta)
                : dr_meta_calc_dyn_size(keep_data.m_result_meta, keep_data.m_result_count_init);
            if (data_capacity < 0) {
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: create result buff, calc capacity fail, meta=%s, count=%d!",
                    mongo_cli_proxy_name(proxy), dr_meta_name(keep_data.m_result_meta), (int)keep_data.m_result_count_init);
                parse_result = mongo_cli_pkg_parse_fail;
                goto RECV_COMPLETE;
            }

            result_data = logic_require_data_get_or_create(require, keep_data.m_result_meta, data_capacity);
            if (result_data == NULL) {
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: create result buff fail, capacity=%d!",
                    mongo_cli_proxy_name(proxy), (int)data_capacity);
                parse_result = mongo_cli_pkg_parse_fail;
                goto RECV_COMPLETE;
            }
        }

        if (result_data) {
            result_meta = logic_data_record_meta(result_data);
            if (result_meta == NULL) {
                CPE_ERROR(proxy->m_em, "%s: recv_response: get result meta fail!", mongo_cli_proxy_name(proxy));
                parse_result = mongo_cli_pkg_parse_fail;
                goto RECV_COMPLETE;
            }
        }

        result_prefix = keep_data.m_prefix[0] ? keep_data.m_prefix : NULL;

        if (keep_data.m_cmd == MONGO_CMD_FINDANDMODIFY) {
            parse_result = mongo_cli_proxy_recv_process_find_and_modify(proxy, pkg, require, &result_data, result_meta, result_prefix, &keep_data);
            goto RECV_COMPLETE;
        }
        else {
            bson_iter_t bson_err;
            bson_iter_t bson_code;

            if (result_meta == proxy->m_meta_lasterror) {
                parse_result = mongo_cli_proxy_recv_process_find_last_error(proxy, pkg, require, result_data);
                goto RECV_COMPLETE;
            }

            if (mongo_pkg_find(&bson_err, pkg, 0, "$err") == 0) {
                if (mongo_pkg_find(&bson_code, pkg, 0, "code") == 0) {
                    int32_t err = bson_iter_int32(&bson_code);
                    if (proxy->m_debug) {
                        CPE_INFO(
                            proxy->m_em, "%s: recv_response: query: error: %s, (code=%d)!",
                            mongo_cli_proxy_name(proxy), bson_iter_utf8(&bson_err, NULL), err);
                    }
                    parse_result = mongo_cli_pkg_parse_fail;
                    goto RECV_COMPLETE;
                }
                else {
                    if (proxy->m_debug) {
                        CPE_INFO(
                            proxy->m_em, "%s: recv_response: query: error: %s, (code=?)!",
                            mongo_cli_proxy_name(proxy), bson_iter_utf8(&bson_err, NULL));
                    }
                    parse_result = mongo_cli_pkg_parse_fail;
                    goto RECV_COMPLETE;
                }
            }

            parse_result = mongo_cli_proxy_recv_build_results(proxy, pkg, require, &result_data, result_meta, result_prefix, &keep_data);
            if (parse_result != mongo_cli_pkg_parse_next) goto RECV_COMPLETE;
        }
    }

RECV_COMPLETE:
    if (parse_result == mongo_cli_pkg_parse_next) {
        if (keep_data.m_parser) {
            parse_result = keep_data.m_parser(
                keep_data.m_parse_ctx, require, pkg, &result_data, keep_data.m_result_meta,
                mongo_cli_pkg_parse_end, NULL, 0);

            switch(parse_result) {
            case mongo_cli_pkg_parse_fail:
            case mongo_cli_pkg_parse_success:
                break;
            case mongo_cli_pkg_parse_next:
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: on_parse_end return next!",
                    mongo_cli_proxy_name(proxy));
                parse_result = mongo_cli_pkg_parse_fail;
            default:
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: on_parse_end unknown parse result %d!",
                    mongo_cli_proxy_name(proxy), parse_result);
                parse_result = mongo_cli_pkg_parse_fail;
            }
        }
        else {
            parse_result = mongo_cli_pkg_parse_success;
        }
    }

    assert(parse_result != mongo_cli_pkg_parse_next);

    if (parse_result == mongo_cli_pkg_parse_success) {
        logic_require_set_done(require);
    }
    else {
        if (logic_require_state(require) != logic_require_state_error) {
            logic_require_set_error(require);
        }
    }

    return 0;
}
