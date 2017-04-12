#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"

int conn_svr_ss_trans_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_svr_t svr = ctx;
    dp_req_t req_head;
    dp_req_t req_carry;
    CONN_SVR_CONN_INFO * pkg_carry_data;
    conn_svr_conn_t conn;

    req_head = set_pkg_head_find(req);
    if (req_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process ss response: find head fail!", conn_svr_name(svr));
        return -1;
    }

    req_carry = set_pkg_carry_find(req);
    if (req_carry == NULL) {
        CPE_ERROR(svr->m_em, "%s: process ss response: find carry fail!", conn_svr_name(svr));
        return -1;
    }

    if (set_pkg_carry_size(req_carry) != sizeof(CONN_SVR_CONN_INFO)) {
        CPE_ERROR(
            svr->m_em, "%s: response(svr_type=%d, svr_id=%d): carray data len error, len=%d!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
            (int)set_pkg_carry_size(req_carry));
        return -1;
    }

    pkg_carry_data = set_pkg_carry_data(req_carry);

    if (pkg_carry_data->conn_id) {
        conn = conn_svr_conn_find_by_conn_id(svr, pkg_carry_data->conn_id);
        if (conn == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: response(svr_type=%d, svr_id=%d): connection %d not exist!",
                conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
                pkg_carry_data->conn_id);
            return -1;
        }

        if (conn->m_data.user_id != 0 && pkg_carry_data->user_id != 0 && conn->m_data.user_id != pkg_carry_data->user_id) {
            CPE_ERROR(
                svr->m_em, "%s: response(svr_type=%d, svr_id=%d): conn(conn_id=%d, fd=%d) user id mismatch "FMT_UINT64_T" and "FMT_UINT64_T"!",
                conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
                pkg_carry_data->conn_id, conn->m_fd, conn->m_data.user_id, pkg_carry_data->user_id);
            return -1;
        }
    }
    else if (pkg_carry_data->user_id) {
        conn = conn_svr_conn_find_by_user_id(svr, pkg_carry_data->user_id);
        if (conn == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: response(svr_type=%d, svr_id=%d): no connection bind with user "FMT_UINT64_T"!",
                conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
                pkg_carry_data->user_id);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: response(svr_type=%d, svr_id=%d): no user_id or conn_id!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head));
        return -1;
    }

    assert(conn);

    if (conn_svr_conn_net_send(conn, set_pkg_from_svr_type(req_head), 0, set_pkg_sn(req_head), dp_req_data(req), dp_req_size(req), dp_req_meta(req)) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: response(svr_type=%d, svr_id=%d): send data at conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T") fail!",
            conn_svr_name(svr), set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head),
            conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
        return -1;
    }
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): send one response(svr_type=%d, svr_id=%d)!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id,
            set_pkg_from_svr_type(req_head), set_pkg_from_svr_id(req_head));
    }

    return 0;
}
