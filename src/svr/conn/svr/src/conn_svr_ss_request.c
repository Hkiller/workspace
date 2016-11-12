#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"
#include "protocol/svr/conn/svr_conn_pro.h"

static conn_svr_op_t g_svr_ops[] = {
    NULL
    , conn_svr_op_bind_user, NULL
    , conn_svr_op_close, NULL
};


int conn_svr_ss_request_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_svr_t svr = ctx;
    SVR_CONN_PKG * pkg;

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_ops) / sizeof(g_svr_ops[0]))
        || g_svr_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process: not support cmd %d!", conn_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_ops[pkg->cmd](svr, req);

    return 0;
}
