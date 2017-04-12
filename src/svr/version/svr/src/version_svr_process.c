#include "cpe/dp/dp_request.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/share/set_pkg.h"
#include "version_svr_ops.h"

static version_svr_op_t g_svr_ops[] = {
    NULL
    , version_svr_op_query, NULL
};

int version_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    version_svr_t svr = ctx;
    dp_req_t pkg_head;
    SVR_VERSION_PKG * pkg;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process: find pkg head fail!", version_svr_name(svr));
        return -1;
    }

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_ops) / sizeof(g_svr_ops[0]))
        || g_svr_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process: not support cmd %d!", version_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_ops[pkg->cmd](svr, pkg_head, req);

    return 0;
}

void version_svr_send_error_response(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err) {
    SVR_VERSION_RES_ERROR pkg;

    if (set_pkg_sn(pkg_head) == 0) return;

    pkg.error = err;

    if (set_svr_stub_reply_data(svr->m_stub, pkg_body, &pkg, sizeof(pkg), svr->m_meta_res_error) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_error_response: send fail!", version_svr_name(svr));
        return;
    }
}
