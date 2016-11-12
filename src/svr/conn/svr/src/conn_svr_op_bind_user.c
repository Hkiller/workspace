#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"

void conn_svr_op_bind_user(conn_svr_t svr, dp_req_t agent_pkg) {
    SVR_CONN_REQ_BIND_USER * req;
    conn_svr_conn_t conn = NULL;

    req = &((SVR_CONN_PKG*)dp_req_data(agent_pkg))->data.svr_conn_req_bind_user;

    conn = conn_svr_conn_find_by_conn_id(svr, req->data.conn_id);
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: bind_user: conn %d not exist!", conn_svr_name(svr), req->data.conn_id);
        return;
    }

    if (req->data.user_id) {
        conn_svr_conn_t old_conn = conn_svr_conn_find_by_user_id(svr, req->data.user_id);
        if (old_conn) {
            CPE_INFO(
                svr->m_em, "%s: bind_user: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): close old conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T")!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id,
                old_conn->m_data.conn_id, old_conn->m_fd, old_conn->m_data.user_id);
            conn_svr_conn_free(old_conn);
        }
    }

    if (conn_svr_conn_set_user_info(conn, &req->data) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: bind_user: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T") set user "FMT_UINT64_T" fail!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, req->data.user_id);
        return;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: bind_user: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T") set user "FMT_UINT64_T" success!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, req->data.user_id);
    }
}
