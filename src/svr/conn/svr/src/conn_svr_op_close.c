#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"

void conn_svr_op_close(conn_svr_t svr, dp_req_t agent_pkg) {
    SVR_CONN_REQ_CLOSE * req;
    conn_svr_conn_t conn = NULL;

    req = &((SVR_CONN_PKG*)dp_req_data(agent_pkg))->data.svr_conn_req_close;

    if (req->user_id) {
        conn = conn_svr_conn_find_by_user_id(svr, req->user_id);
        if (conn == NULL) {
            CPE_ERROR(svr->m_em, "%s: close: conn bind with user "FMT_UINT64_T" not exist!", conn_svr_name(svr), req->user_id);
            return;
        }

        if (req->conn_id && req->conn_id != conn->m_data.conn_id) {
            CPE_ERROR(svr->m_em, "%s: close: conn bind with user "FMT_UINT64_T" not exist!", conn_svr_name(svr), req->user_id);
            return;
        }
    }
    else if (req->conn_id) {
        conn = conn_svr_conn_find_by_conn_id(svr, req->conn_id);
        if (conn == NULL) {
            CPE_ERROR(svr->m_em, "%s: close: conn %d not exist!", conn_svr_name(svr), req->conn_id);
            return;
        }
    }
    else {
        CPE_ERROR(svr->m_em, "%s: close: no user_id or conn_id set in req!", conn_svr_name(svr));
        return;
    }

    assert(conn);

    conn_svr_conn_free(conn);
}
