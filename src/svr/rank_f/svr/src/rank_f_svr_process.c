#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "rank_f_svr_ops.h"

static rank_f_svr_op_t g_svr_rank_f_request_ops[] = {
    NULL
    , rank_f_svr_request_update, NULL
    , rank_f_svr_request_remove, NULL
    , rank_f_svr_request_clear, NULL
    , rank_f_svr_request_query, NULL
    , rank_f_svr_request_query_with_data, NULL
};

int rank_f_svr_request_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    rank_f_svr_t svr = ctx;
    dp_req_t pkg_head;
    SVR_RANK_F_PKG * pkg;

    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process rank_f request: no pkg head!", rank_f_svr_name(svr));
        return -1;
    }

    pkg = dp_req_data(req);

    if (pkg->cmd >= (sizeof(g_svr_rank_f_request_ops) / sizeof(g_svr_rank_f_request_ops[0]))
        || g_svr_rank_f_request_ops[pkg->cmd] == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: process rank_f request: not support cmd %d!", rank_f_svr_name(svr), pkg->cmd);
        return -1;
    }
    
    g_svr_rank_f_request_ops[pkg->cmd](svr, req, pkg_head);

    return 0;
}
