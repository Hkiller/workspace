#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "protocol/svr/conn/svr_conn_pro.h"
#include "version_svr_ops.h"
#include "version_svr_version.h"
#include "version_svr_package.h"

static int version_svr_op_query_build_result(
    version_svr_t svr, SVR_VERSION_PACKAGE_WITH_VERSION * r_version, version_svr_version_t version, CONN_SVR_CONN_INFO const * pkg_carry_data);

void version_svr_op_query(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    SVR_VERSION_REQ_QUERY const * req;
    SVR_VERSION_RES_QUERY * response;
    dp_req_t response_pkg;
    version_svr_version_t from_version;
    version_svr_version_t to_version;
    version_svr_version_t process_version;
    version_svr_package_t package;
    uint16_t package_count;
    uint8_t update_strategy = SVR_VERSION_UPDATE_STRATEGY_NO;
    uint8_t included_from_version = 0;
    dp_req_t req_carry;
    CONN_SVR_CONN_INFO * pkg_carry_data;

    req_carry = set_pkg_carry_find(pkg_body);
    if (req_carry == NULL) {
        CPE_ERROR(svr->m_em, "%s: version_svr_op_query: find conn info fail!", version_svr_name(svr));
        version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_INTERNAL);
        return;
    }

    if (set_pkg_carry_size(req_carry) != sizeof(CONN_SVR_CONN_INFO)) {
        CPE_ERROR(svr->m_em, "%s: version_svr_op_query: conn info size error!", version_svr_name(svr));
        version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_INTERNAL);
        return;
    }

    pkg_carry_data = set_pkg_carry_data(req_carry);

    
    req = &((SVR_VERSION_PKG*)dp_req_data(pkg_body))->data.svr_version_req_query;

    to_version = version_svr_version_find(svr, req->to_version);
    if (to_version == NULL) {
        CPE_ERROR(svr->m_em, "%s: version_svr_op_query: target version %s unknown!", version_svr_name(svr), req->to_version);
        version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_VERSION_UNKNOWN);
        return;
    }

    package_count = 0;
    from_version = version_svr_version_find(svr, req->from_version);
    if (from_version == NULL) {
        TAILQ_FOREACH(process_version, &svr->m_versions, m_next) {
            if (process_version->m_update_strategy == SVR_VERSION_UPDATE_STRATEGY_NO) break;

            if ((package = version_svr_package_find(process_version, pkg_carry_data->chanel, pkg_carry_data->device_category)) == NULL) {
                from_version = NULL;
            }
            else {
                if (package->m_data.type == svr_version_package_type_full) from_version = process_version;
            }
        }

        if (from_version == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: version_svr_op_query: from version %s unknown, and no full package to start from!",
                version_svr_name(svr), req->from_version);
            version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_NO_PATH);
            return;
        }

        included_from_version = 1;
    }

    if(from_version->m_update_strategy == SVR_VERSION_UPDATE_STRATEGY_NO) {
        package_count = 0;
    }
    else {
        version_svr_version_t disconnect_version = NULL;
        process_version = from_version;
        while(process_version != to_version) {
            version_svr_version_t next_version = TAILQ_NEXT(process_version, m_next);
            if (next_version == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: version_svr_op_query: no path from %s to %s!",
                    version_svr_name(svr), from_version->m_name, to_version->m_name);
                version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_NO_PATH);
                return;
            }

            if (next_version->m_update_strategy == SVR_VERSION_UPDATE_STRATEGY_NO) {
                to_version = process_version;
                break;
            }

            process_version = next_version;
            if ((package = version_svr_package_find(process_version, pkg_carry_data->chanel, pkg_carry_data->device_category)) == NULL) {
                update_strategy = svr_version_package_type_full;
                from_version = NULL;
                if (disconnect_version == NULL) disconnect_version = process_version;
            }
            else {
                if (process_version->m_update_strategy > update_strategy) update_strategy = process_version->m_update_strategy;
        
                if (package->m_data.type == svr_version_package_type_full) {
                    from_version = process_version;
                    package_count = 0;
                    included_from_version = 1;
                }
                else {
                    package_count++;
                }
            }
        }

        if (from_version == NULL) {
            assert(disconnect_version);
            CPE_ERROR(
                svr->m_em, "%s: version_svr_op_query: version %s no package of chanel %s, device_category=%d!",
                version_svr_name(svr), disconnect_version->m_name, pkg_carry_data->chanel, pkg_carry_data->device_category);
            version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_INTERNAL);
            return;
        }
        
        if (included_from_version) package_count++;
    }
    
    /*构造返回数据 */
    response_pkg = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_VERSION_PKG) + sizeof(SVR_VERSION_PACKAGE) * package_count);
    if (response_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: version_svr_op_query: get response pkg fail, size=%d!",
            version_svr_name(svr), (int)(sizeof(SVR_VERSION_PKG) + sizeof(SVR_VERSION_PACKAGE) * package_count));
        version_svr_send_error_response(svr, pkg_head, pkg_body, SVR_VERSION_ERRNO_INTERNAL);
        return;
    }

    response = set_svr_stub_pkg_to_data(svr->m_stub, response_pkg, 0, svr->m_meta_res_query, NULL);
    assert(response);

    response->update_strategy = update_strategy;
    response->package_count = package_count;

    if (package_count > 0) {
        uint16_t i = 0;
        int rv;

        if (included_from_version) {
            assert(from_version);

            if ((rv = version_svr_op_query_build_result(svr, &response->packages[i++], from_version, pkg_carry_data))) {
                version_svr_send_error_response(svr, pkg_head, pkg_body, rv);
                return;
            }

            from_version = TAILQ_NEXT(from_version, m_next);
        }
    
        for(; i < package_count; ++i) {
            assert(from_version);

            if ((rv = version_svr_op_query_build_result(svr, &response->packages[i], from_version, pkg_carry_data))) {
                version_svr_send_error_response(svr, pkg_head, pkg_body, rv);
                return;
            }

            from_version = TAILQ_NEXT(from_version, m_next);
        }
    }

    if (set_svr_stub_reply_pkg(svr->m_stub, pkg_body, response_pkg) != 0) {
        CPE_ERROR(svr->m_em, "%s: version_svr_op_query: send response fail!", version_svr_name(svr));
        return;
    }
}

static int version_svr_op_query_build_result(
    version_svr_t svr, SVR_VERSION_PACKAGE_WITH_VERSION * r_version, version_svr_version_t version, CONN_SVR_CONN_INFO const * pkg_carry_data)
{
    version_svr_package_t package;

    cpe_str_dup(r_version->version, sizeof(r_version->version), version->m_name);

    package = version_svr_package_find(version, pkg_carry_data->chanel, pkg_carry_data->device_category);
    if (package == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: version_svr_op_query: build result: version %s no package of chanel %s, device_category=%d!",
            version_svr_name(svr), version->m_name, pkg_carry_data->chanel, pkg_carry_data->device_category);
        return -1;
    }
    
    r_version->package = package->m_data;

    return 0;
}
