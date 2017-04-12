#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_svr_adapter_damai.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"
#include "payment_svr_waiting.h"

int payment_svr_damai_init(payment_svr_adapter_t adapter, cfg_t cfg) {
    struct payment_svr_adapter_damai * damai = (struct payment_svr_adapter_damai *)adapter->m_private;
    
    assert(sizeof(*damai) <= sizeof(adapter->m_private));

    damai->m_timeout_s = cfg_get_uint32(cfg, "timeout-s", 300);
    
    return 0;
}

void payment_svr_damai_fini(payment_svr_adapter_t adapter) {
}

int payment_svr_damai_charge_send(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_damai * damai = (struct payment_svr_adapter_damai *)adapter->m_private;
    logic_require_t require;

    /*如果没有外部ID，表示主动通知没有收到，尝试等待通知 */
    if (record->vendor_record.damai.orderid[0] == 0) {
        if (record->commit_time > 0 && payment_svr_cur_time(svr) > (record->commit_time + damai->m_timeout_s)) {
            record->state = PAYMENT_RECHARGE_TIMEOUT;
            record->error = 0;
            record->error_msg[0] = 0;

            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: damai: request %s commit at %d, cur-time=%d, timeout(timeout-s=%d)!",
                    payment_svr_name(svr), record->_id, record->commit_time, payment_svr_cur_time(svr), damai->m_timeout_s);
            }

            return 0;
        }
        
        require = logic_require_create(stack, "damai_wait_notify");
            
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: damai: create require fail", payment_svr_name(svr));
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        if (payment_svr_waiting_start(svr, record->_id, require) != 0) {
            CPE_ERROR(svr->m_em, "%s: damai: start waiting notify of trade %s fail", payment_svr_name(svr), record->_id);
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        return 0;
    }

    record->state = PAYMENT_RECHARGE_SUCCESS;
    record->error = 0;
    record->error_msg[0] = 0;

    return 0;
}

static int payment_svr_damai_charge_recv_waiting(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);

int payment_svr_damai_charge_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;

    if (strcmp(logic_require_name(require), "damai_wait_notify") == 0) {
        return payment_svr_damai_charge_recv_waiting(ctx, stack, require, adapter, svr, record, req);
    }
    else {
        CPE_ERROR(svr->m_em, "%s: damai: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    return 0;
}

static int payment_svr_damai_charge_recv_waiting(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    PAYMENT_RECHARGE_RECORD * waiting_record;
    
    payment_svr_waiting_stop(svr, record->_id, require);
    
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            logic_context_errno_set(ctx, logic_require_error(require));
        }
        else if (logic_require_state(require) == logic_require_state_timeout) {
            if (svr->m_debug) {
                CPE_INFO(svr->m_em, "%s: damai: waiting notify timeout!", payment_svr_name(svr));
            }
            return 0;
        }
        else {
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        }
        return -1;
    }

    waiting_record = logic_data_data(logic_require_data_find(require, dr_meta_name(svr->m_meta_recharge_record)));
    if (waiting_record->version != record->version + 1) {
        CPE_ERROR(
            svr->m_em, "%s: damai: waiting record version error, record-version=%d, waiting-record-version=%d!",
            payment_svr_name(svr), record->version, waiting_record->version);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    * record = * waiting_record;

    if (record->vendor_record.damai.orderid[0] == 0) {
        CPE_ERROR(svr->m_em, "%s: damai: wait success, but damai order id still 0!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    return payment_svr_damai_charge_send(ctx, stack, adapter, record, req);
}
