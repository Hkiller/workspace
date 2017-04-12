#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/utils/id_generator.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_use/id_generator.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "mail_svr_ops.h"
#include "protocol/svr/mail/svr_mail_pro.h"
#include "protocol/svr/mail/svr_mail_internal.h"

logic_op_exec_result_t
mail_svr_op_send_global_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    mail_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_MAIL_REQ_SEND_GLOBAL_MAIL const * req;
    int rv;
    char buf[svr->m_record_global_size];
    SVR_MAIL_RECORD_GLOBAL_TMPL * record = (SVR_MAIL_RECORD_GLOBAL_TMPL *)buf;

    bzero(buf, sizeof(buf));

    req_data = logic_context_data_find(ctx, "svr_mail_req_send_global_mail");
    if(req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: send global mail: read record: find req fail", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    req = logic_data_data(req_data);

    cpe_str_dup(record->title, sizeof(record->title), req->title);
    cpe_str_dup(record->body, sizeof(record->body), req->body);
    record->send_time = mail_svr_cur_time(svr);
    record->state = req->state;

    rv = dr_pbuf_read(
        buf + svr->m_record_global_attach_start_pos,
        svr->m_record_global_attach_capacity,
        req->attach,
        req->attach_len,
        svr->m_attach_global_meta,
        svr->m_em);
    if (rv <= 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: decode attach error, rv=%d!", mail_svr_name(svr), rv);
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (gd_id_generator_generate(&record->_id, (gd_id_generator_t)svr->m_id_generator, "mail_id") != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: gen global mail id fail!", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    require = logic_require_create(stack, "send_global");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: create send_global require fail!", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (mail_svr_db_send_global_insert(svr, require, buf) != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: db insert error", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
mail_svr_op_send_global_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    mail_svr_t svr = user_data;

    if (logic_require_state(require) != logic_require_state_done) {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: add: db request error, state=%s, errno=%d!",
            mail_svr_name(svr), logic_require_state_name(logic_require_state(require)), logic_require_error(require));
        return logic_op_exec_result_false;
    }
    return logic_op_exec_result_true;
}
