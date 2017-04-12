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
mail_svr_op_query_global_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    mail_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_MAIL_REQ_QUERY_MAIL_FULL const * req;

    req_data = logic_context_data_find(ctx, "svr_mail_req_query_mail_full");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: get request fail!", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    req = logic_data_data(req_data);

    require = logic_require_create(stack, "query_global");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: query_global: create logic require fail!", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (mail_svr_db_send_query_global(svr, require, req->create_time) != 0) {
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
mail_svr_op_query_global_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    mail_svr_t svr = user_data;
    logic_data_t query_result_data;
    SVR_MAIL_RECORD_GLOBAL_LIST * query_result;

    if (logic_require_state(require) != logic_require_state_done) {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: add: db request error, state=%s, errno=%d!",
            mail_svr_name(svr), logic_require_state_name(logic_require_state(require)), logic_require_error(require));
        return logic_op_exec_result_false;
    }

    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_record_global_list_meta));
    assert(query_result_data);
    query_result = logic_data_data(query_result_data);

    if(query_result->record_count == 0) {
        CPE_INFO(svr->m_em, "%s: global mail not exit", mail_svr_name(svr));
        return logic_op_exec_result_true;
    }

    return logic_op_exec_result_true;
}
