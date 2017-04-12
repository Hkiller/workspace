#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_svr_adapter_qihoo.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"
#include "payment_svr_waiting.h"

static int payment_svr_qihoo_data_init(payment_svr_t svr, struct payment_svr_adapter_qihoo_data * data, cfg_t cfg);
static void payment_svr_qihoo_data_fini(payment_svr_t svr, struct payment_svr_adapter_qihoo_data * data);
static struct payment_svr_adapter_qihoo_data *
payment_svr_adapter_qihoo_env(payment_svr_t svr, payment_svr_adapter_t adapter, PAYMENT_RECHARGE_RECORD * record);

int payment_svr_qihoo_init(payment_svr_adapter_t adapter, cfg_t cfg) {
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_qihoo * qihoo = (struct payment_svr_adapter_qihoo *)adapter->m_private;
    cfg_t platform_cfg;
    
    assert(sizeof(*qihoo) <= sizeof(adapter->m_private));

    qihoo->m_trans_group = net_trans_group_create(svr->m_trans_mgr, "qihoo");
    if (qihoo->m_trans_group == NULL) {
        CPE_ERROR(svr->m_em, "%s: qihoo: crate trans group fail!", payment_svr_name(svr));
        return -1;
    }

    qihoo->m_timeout_s = cfg_get_uint32(cfg, "timeout-s", 300);
    
    if ((platform_cfg = cfg_find_cfg(cfg, "android"))) {
        if (payment_svr_qihoo_data_init(svr, &qihoo->m_android, platform_cfg) != 0) {
            net_trans_group_free(qihoo->m_trans_group);
            return -1;
        }
    }
    
    return 0;
}

void payment_svr_qihoo_fini(payment_svr_adapter_t adapter) {
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_qihoo * qihoo = (struct payment_svr_adapter_qihoo *)adapter->m_private;

    payment_svr_qihoo_data_fini(svr, &qihoo->m_android);

    net_trans_group_free(qihoo->m_trans_group);
    qihoo->m_trans_group = NULL;
}

static int payment_svr_qihoo_data_init(payment_svr_t svr, struct payment_svr_adapter_qihoo_data * data, cfg_t cfg) {
    const char * appid = cfg_get_string(cfg, "appid", NULL);
    const char * appkey = cfg_get_string(cfg, "appkey", NULL);
    const char * appsecret = cfg_get_string(cfg, "appsecret", NULL);

    if (appid == NULL || appkey == NULL || appkey == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: qihoo: config error, appid=%s, appkey=%s, appsecret=%s", appid, appkey, appsecret);
        return -1;
    }

    data->m_appid = cpe_str_mem_dup(svr->m_alloc, appid);
    data->m_appkey = cpe_str_mem_dup(svr->m_alloc, appkey);
    data->m_appsecret = cpe_str_mem_dup(svr->m_alloc, appsecret);
    if (data->m_appid == NULL || data->m_appkey == NULL || data->m_appsecret == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: qihoo: load config alloc fail!");

        if (data->m_appid) { mem_free(svr->m_alloc, data->m_appid); data->m_appid = NULL; }
        if (data->m_appkey) { mem_free(svr->m_alloc, data->m_appkey); data->m_appkey = NULL; }
        if (data->m_appsecret) { mem_free(svr->m_alloc, data->m_appsecret); data->m_appsecret = NULL; }

        return -1;
    }

    return 0;
}

static void payment_svr_qihoo_data_fini(payment_svr_t svr, struct payment_svr_adapter_qihoo_data * data) {
    if (data->m_appid) { mem_free(svr->m_alloc, data->m_appid); data->m_appid = NULL; }
    if (data->m_appkey) { mem_free(svr->m_alloc, data->m_appkey); data->m_appkey = NULL; }
    if (data->m_appsecret) { mem_free(svr->m_alloc, data->m_appsecret); data->m_appsecret = NULL; }
}

int payment_svr_qihoo_charge_send(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_qihoo * qihoo = (struct payment_svr_adapter_qihoo *)adapter->m_private;
    struct payment_svr_adapter_qihoo_data * env;
    logic_require_t require;

    env = payment_svr_adapter_qihoo_env(svr, adapter, record);
    if (env == NULL) {
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    /*如果没有外部ID，表示主动通知没有收到，尝试等待通知 */
    if (record->vendor_record.qihoo.order_id == 0) {
        if (record->commit_time > 0 && payment_svr_cur_time(svr) > (record->commit_time + qihoo->m_timeout_s)) {
            record->state = PAYMENT_RECHARGE_TIMEOUT;
            record->error = 0;
            record->error_msg[0] = 0;

            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: qihoo: request %s commit at %d, cur-time=%d, timeout(timeout-s=%d)!",
                    payment_svr_name(svr), record->_id, record->commit_time, payment_svr_cur_time(svr), qihoo->m_timeout_s);
            }

            return 0;
        }
        
        require = logic_require_create(stack, "qihoo_wait_notify");
            
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: qihoo: create require fail", payment_svr_name(svr));
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        if (payment_svr_waiting_start(svr, record->_id, require) != 0) {
            CPE_ERROR(svr->m_em, "%s: qihoo: start waiting notify of trade %s fail", payment_svr_name(svr), record->_id);
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        return 0;
    }

    if (strcmp(record->vendor_record.qihoo.gateway_flag, "success") == 0) {
        record->state = PAYMENT_RECHARGE_SUCCESS;
        record->error = 0;
        record->error_msg[0] = 0;
    }
    else {
        record->state = PAYMENT_RECHARGE_FAIL;
        record->error = 0;
        cpe_str_dup(record->error_msg, sizeof(record->error_msg), record->vendor_record.qihoo.gateway_flag);
    }

    return 0;
}

static int payment_svr_qihoo_charge_recv_waiting(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);

int payment_svr_qihoo_charge_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;

    if (strcmp(logic_require_name(require), "qihoo_wait_notify") == 0) {
        return payment_svr_qihoo_charge_recv_waiting(ctx, stack, require, adapter, svr, record, req);
    }
    else {
        CPE_ERROR(svr->m_em, "%s: qihoo: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    return 0;
}

static int payment_svr_qihoo_charge_recv_waiting(
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
                CPE_INFO(svr->m_em, "%s: qihoo: waiting notify timeout!", payment_svr_name(svr));
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
            svr->m_em, "%s: qihoo: waiting record version error, record-version=%d, waiting-record-version=%d!",
            payment_svr_name(svr), record->version, waiting_record->version);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    * record = * waiting_record;

    if (record->vendor_record.qihoo.order_id == 0) {
        CPE_ERROR(svr->m_em, "%s: qihoo: wait success, but qihoo order id still 0!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    return payment_svr_qihoo_charge_send(ctx, stack, adapter, record, req);
}

static
struct payment_svr_adapter_qihoo_data *
payment_svr_adapter_qihoo_env(payment_svr_t svr, payment_svr_adapter_t adapter, PAYMENT_RECHARGE_RECORD * record) {
    struct payment_svr_adapter_qihoo * qihoo = (struct payment_svr_adapter_qihoo *)adapter->m_private;
    
    switch(record->device_category) {
    case svr_payment_device_windows:
        CPE_ERROR(svr->m_em, "%s: qihoo: not support device windows", payment_svr_name(svr));
        return NULL;
    case svr_payment_device_ios:
        CPE_ERROR(svr->m_em, "%s: qihoo: not support device ios", payment_svr_name(svr));
        return NULL;
    case svr_payment_device_android:
        return &qihoo->m_android;
    default:
        CPE_ERROR(
            svr->m_em, "%s: qihoo: unknown device category %d",
            payment_svr_name(svr), record->device_category);
        return NULL;
    }
}
