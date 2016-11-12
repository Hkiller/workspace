#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

void rank_f_svr_request_clear(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_F_REQ_CLEAR * req;

    req = &((SVR_RANK_F_PKG*)dp_req_data(pkg_body))->data.svr_rank_f_req_clear;

    rank_f_svr_user_destory(svr, req->user_id);

    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_send_response_cmd(
                svr->m_stub,
                set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
                SVR_RANK_F_CMD_RES_CLEAR, NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: request clear: send response fail!", rank_f_svr_name(svr));
        }
    }
}
