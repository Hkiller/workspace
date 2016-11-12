#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "conn_http_svr_ops.h"

static conn_http_svr_op_t g_svr_conn_http_request_ops[] = {
    NULL
};

int conn_http_request_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_http_svr_t svr = ctx;
    dp_req_t pkg_head;
    SVR_CONN_HTTP_PKG * pkg;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process request: no pkg head!", conn_http_svr_name(svr));
        return -1;
    }

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_conn_http_request_ops) / sizeof(g_svr_conn_http_request_ops[0]))
        || g_svr_conn_http_request_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process request: not support cmd %d!", conn_http_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_conn_http_request_ops[pkg->cmd](svr, req, pkg_head);

    return 0;
}
