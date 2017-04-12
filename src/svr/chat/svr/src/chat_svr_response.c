#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "chat_svr_ops.h"

void chat_svr_send_error_response(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err) {
    SVR_CHAT_RES_ERROR pkg;

    if (set_pkg_sn(pkg_head) == 0) return;

    pkg.error = err;

    if (set_svr_stub_reply_data(svr->m_stub, pkg_body, &pkg, sizeof(pkg), svr->m_meta_res_error) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_error_response: send fail!", chat_svr_name(svr));
        return;
    }
}
