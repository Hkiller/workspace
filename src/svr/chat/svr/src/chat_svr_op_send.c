#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "chat_svr_ops.h"

void chat_svr_op_send(chat_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body) {
    SVR_CHAT_REQ_SEND_MSG const * req;
    SVR_CHAT_MSG * msg;
    chat_svr_chanel_t chanel;

    req = &((SVR_CHAT_PKG*)dp_req_data(pkg_body))->data.svr_chat_req_send_msg;

    chanel = chat_svr_chanel_find(svr, req->chanel_type, req->chanel_id);
    if (chanel == NULL) {
        SVR_CHAT_CHANEL_INFO const * chanel_info
            = chat_svr_meta_chanel_find(svr, req->chanel_type);
        if (chanel_info == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: chanel %d-"FMT_UINT64_T": send msg: chanel type unknown!",
                chat_svr_name(svr), req->chanel_type, req->chanel_id);
            chat_svr_send_error_response(svr, pkg_head, pkg_body, SVR_CHAT_ERRNO_CHANEL_TYPE_UNKNOWN);
            return;
        }
        
        chanel = chat_svr_chanel_create(svr, chanel_info, req->chanel_id);
        if (chanel == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: chanel %d-"FMT_UINT64_T": send msg: create chanel error!",
                chat_svr_name(svr), req->chanel_type, req->chanel_id);
            chat_svr_send_error_response(svr, pkg_head, pkg_body, SVR_CHAT_ERRNO_INTERNAL);
            return;
        }
    }

    assert(chanel);

    chanel->m_last_op_time_s = chat_svr_cur_time(svr);

    msg = chat_svr_chanel_append_msg(chanel);
    if (msg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: chanel %d-"FMT_UINT64_T": send msg: append msg fail!",
            chat_svr_name(svr), chanel->m_chanel_type, chanel->m_chanel_id);
        chat_svr_send_error_response(svr, pkg_head, pkg_body, SVR_CHAT_ERRNO_INTERNAL);
        return;
    }

    msg->send_time = chat_svr_cur_time(svr);
    msg->sender_id = req->sender_id;
    cpe_str_dup(msg->sender_name, sizeof(msg->sender_name), req->sender_name);
    cpe_str_dup(msg->msg, sizeof(msg->msg), req->msg);

    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_reply_cmd(svr->m_stub, pkg_body, SVR_CHAT_CMD_RES_SEND_MSG) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: chanel %d-"FMT_UINT64_T": send msg: send response fail!",
                chat_svr_name(svr), chanel->m_chanel_type, chanel->m_chanel_id);
            return;
        }
    }

    return;
}
