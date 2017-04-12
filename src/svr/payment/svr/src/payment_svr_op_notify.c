#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/time_utils.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "payment_svr_ops.h"
#include "payment_svr_db_recharge_ops.h"
#include "payment_svr_db_ops.h"
#include "payment_svr_waiting.h"

logic_op_exec_result_t
payment_svr_op_notify_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_NOTIFY const * req;
    logic_require_t require;
    
    /*获取请求 */
    req_data = logic_context_data_find(ctx, "svr_payment_req_notify");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: notify: get request fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);
    
    /*发送查询请求 */
    require = logic_require_create(stack, "record_query");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: notify: create logic require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    payment_svr_db_recharge_send_qurey_by_id(svr, require, req->trade_id);
    
    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
payment_svr_op_notify_on_record_query(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_t svr);

static logic_op_exec_result_t
payment_svr_op_notify_on_record_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_t svr);

logic_op_exec_result_t
payment_svr_op_notify_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;

    if (strcmp(logic_require_name(require), "record_query") == 0) {
        return payment_svr_op_notify_on_record_query(ctx, stack, require, svr);
    }
    else if (strcmp(logic_require_name(require), "record_update") == 0) {
        return payment_svr_op_notify_on_record_update(ctx, stack, require, svr);
    }
    else {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: notify: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

static logic_op_exec_result_t
payment_svr_op_notify_on_record_query(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_t svr)
{
    int r;
    logic_data_t record_list_data;
    PAYMENT_RECHARGE_RECORD_LIST const * record_list;
    logic_data_t record_data;
    PAYMENT_RECHARGE_RECORD * record;
    SVR_PAYMENT_REQ_NOTIFY const * req;
    logic_require_t update_require;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    req = logic_data_data(logic_context_data_find(ctx, "svr_payment_req_notify"));
    assert(req);
    
    /*构造record */
    record_list_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_recharge_record_list));
    if (record_list_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: notify: on query result: find record list fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    record_list = logic_data_data(record_list_data);

    if (record_list->count == 0) {
        CPE_ERROR(
            svr->m_em, "%s: notify: on query result: record not exist, trade %s not exist!",
            payment_svr_name(svr), req->trade_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_NOT_EXIST);
        return logic_op_exec_result_false;
    }

    record_data = logic_context_data_get_or_create(ctx, svr->m_meta_recharge_record, sizeof(PAYMENT_RECHARGE_RECORD));
    if (record_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: notify: on query result: create record buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    record = logic_data_data(record_data);
    memcpy(record, &record_list->records[0], sizeof(PAYMENT_RECHARGE_RECORD));

    CPE_INFO(
        svr->m_em, "%s: notify: before op: dump record: %s!",
        payment_svr_name(svr), dr_json_dump_inline(gd_app_tmp_buffer(svr->m_app), record, sizeof(*record), svr->m_meta_recharge_record));

    if (record->service != req->service) {
        CPE_ERROR(svr->m_em, "%s: notify: service mismatch record.service=%d, req.service=%d!", payment_svr_name(svr), record->service, req->service);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (record->device_category != req->device_category) {
        CPE_ERROR(
            svr->m_em, "%s: notify: device category mismatch record.device_category=%d, req.device_category=%d!",
            payment_svr_name(svr), record->device_category, req->device_category);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    if(record->state != PAYMENT_RECHARGE_INPROCESS) {
        CPE_ERROR(svr->m_em, "%s: notify: record is not in process!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_PROCESSED);
        return logic_op_exec_result_false;
    }

    /*更新数据 */
    update_require = logic_require_create(stack, "record_update");
    if (update_require == NULL) {
        CPE_ERROR(svr->m_em, "%s: notify: create record_update require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    record->vendor_record = req->data;
    record->version++;
    payment_svr_db_recharge_send_update_state(svr, update_require, record);
    
    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
payment_svr_op_notify_on_record_update(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_t svr) {
    int r;
    PAYMENT_RECHARGE_RECORD * record;
    payment_svr_waiting_t waiting;
    logic_require_t waiting_require;
    logic_data_t waiting_require_data;
    
    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    record = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_recharge_record)));
    assert(record);

    waiting = payment_svr_waiting_find(svr, record->_id);
    if (waiting == NULL) return logic_op_exec_result_true;

    waiting_require = logic_require_find(logic_require_mgr(require), waiting->m_require_id);
    if (waiting_require == NULL) return logic_op_exec_result_true;

    waiting_require_data = logic_require_data_get_or_create(waiting_require, svr->m_meta_recharge_record, sizeof(PAYMENT_RECHARGE_RECORD));
    if (waiting_require_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: notify: on query result: create  record buf on waiting reqruie fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    memcpy(logic_data_data(waiting_require_data), record, sizeof(PAYMENT_RECHARGE_RECORD));
    logic_require_set_done(waiting_require);

    return logic_op_exec_result_true;
}
