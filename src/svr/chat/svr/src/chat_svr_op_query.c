#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "chat_svr_ops.h"

void chat_svr_op_query(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    SVR_CHAT_REQ_QUERY_MSG const * req;
    SVR_CHAT_RES_QUERY_MSG * response;
    uint32_t require_count;
    chat_svr_chanel_t chanel;
    dp_req_t response_pkg;
    uint32_t read_pos;
    uint32_t msg_count;
    SVR_CHAT_MSG * first_msg;

    req = &((SVR_CHAT_PKG*)dp_req_data(pkg_body))->data.svr_chat_req_query_msg;

    require_count = req->require_count;
    if (require_count > 128) require_count = 128;

    
    response_pkg = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_CHAT_PKG) + sizeof(SVR_CHAT_MSG) * require_count);
    if (response_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: chat_svr_op_query: get response pkg fail, size=%d!",
            chat_svr_name(svr), (int)(sizeof(SVR_CHAT_PKG) + sizeof(SVR_CHAT_MSG) * require_count));
        chat_svr_send_error_response(svr, pkg_head, pkg_body, SVR_CHAT_ERRNO_INTERNAL);
        return;
    }

    response = set_svr_stub_pkg_to_data(svr->m_stub, response_pkg, 0, svr->m_meta_res_query, NULL);
    assert(response);

    chanel = chat_svr_chanel_find(svr, req->chanel_type, req->chanel_id);
    if (chanel == NULL) {
        if (chat_svr_meta_chanel_find(svr, req->chanel_type) == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: chanel %d-"FMT_UINT64_T": query: chanel type unknown!",
                chat_svr_name(svr), req->chanel_type, req->chanel_id);
            chat_svr_send_error_response(svr, pkg_head, pkg_body, SVR_CHAT_ERRNO_INTERNAL);
            return;
        }

        goto SEND_RESPONSE;
    }

    chanel->m_last_op_time_s = chat_svr_cur_time(svr);

    if (chanel->m_chanel_msg_w == chanel->m_chanel_msg_r) {
        response->count = 0;
        response->max_sn = 0;
        goto SEND_RESPONSE;
    }

    response->count = 0;
    response->max_sn = chanel->m_chanel_sn;

    read_pos = 0;
    msg_count = chat_svr_chanel_msg_count(chanel);
    first_msg = chat_svr_chanel_msg(chanel, 0);

    if (req->after_sn >= first_msg->sn) {
        uint32_t ignore_count = req->after_sn - first_msg->sn + 1;
        if (ignore_count <= msg_count) {
            read_pos += ignore_count;
        }
    }

    for(; read_pos < msg_count && require_count > 0; ++read_pos, --require_count) {
        memcpy(
            response->msgs + response->count++,
            chat_svr_chanel_msg(chanel, read_pos),
            sizeof(SVR_CHAT_MSG));
    }

SEND_RESPONSE:
    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_reply_pkg(svr->m_stub, pkg_body, response_pkg) != 0) {
            CPE_ERROR(svr->m_em, "%s: chat_svr_op_query: send response fail!", chat_svr_name(svr));
            return;
        }
    }
}
