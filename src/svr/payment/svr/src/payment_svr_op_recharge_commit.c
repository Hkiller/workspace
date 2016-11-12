#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_stack.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "payment_svr_ops.h"
#include "payment_svr_db_ops.h"
#include "payment_svr_db_recharge_ops.h"
#include "payment_svr_db_data_ops.h"
#include "payment_svr_db_bill_ops.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"
#include "payment_svr_meta_bag_info.h"

logic_op_exec_result_t
payment_svr_op_recharge_commit_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req;
    logic_require_t require;
    
    /*获取请求 */
    req_data = logic_context_data_find(ctx, "svr_payment_req_recharge_commit");
    if (req_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: get request fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    require = logic_require_create(stack, "record_query");
    if (require == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: create logic require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    payment_svr_db_recharge_send_qurey_by_id(svr, require, req->trade_id);
        
    return logic_op_exec_result_true;
}

static int payment_svr_op_recharge_commit_build_result(logic_context_t ctx, payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record);
static int payment_svr_op_recharge_commit_check_adapter_complete(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record);
static PAYMENT_BILL_DATA *
payment_svr_op_recharge_build_bill(
    logic_context_t ctx, payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record, BAG_INFO const * bag_info);

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_record_query(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_record_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record);

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_data_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record);

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_data_insert(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record);

logic_op_exec_result_t
payment_svr_op_recharge_commit_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req;

    req = logic_data_data(logic_context_data_find(ctx, "svr_payment_req_recharge_commit"));
    assert(req);

    if (strcmp(logic_require_name(require), "record_query") == 0) {
        /*数据库查询回充值记录 */
        return payment_svr_op_recharge_commit_on_record_query(ctx, stack, require, svr, req);
    }
    else {
        PAYMENT_RECHARGE_RECORD * record;

        record = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_recharge_record)));
        assert(record);
        
        if (strcmp(logic_require_name(require), "record_update") == 0) {
            /*充值记录更新 */
            return payment_svr_op_recharge_commit_on_record_update(ctx, stack, require, svr, record);
        }
        else if (strcmp(logic_require_name(require), "data_update") == 0) {
            /*钱包数据更新 */
            return payment_svr_op_recharge_commit_on_data_update(ctx, stack, require, svr, record);
        }
        else if (strcmp(logic_require_name(require), "data_insert") == 0) {
            /*钱包数据更新，如果记录不存在，则启动插入，处理插入结果 */
            return payment_svr_op_recharge_commit_on_data_insert(ctx, stack, require, svr, record);
        }
        else {
            payment_svr_adapter_t adapter;

            adapter = payment_svr_adapter_find_by_type_id(svr, record->service);
            assert(adapter);

            if (adapter->m_type->m_recv) {
                if (adapter->m_type->m_recv(ctx, stack, require, adapter, record, req) != 0) {
                    return logic_op_exec_result_false;
                }

                if (payment_svr_op_recharge_commit_check_adapter_complete(ctx, stack, svr, record) != 0) {
                    return logic_op_exec_result_false;
                }
                
                return logic_op_exec_result_true;
            }
            else {
                CPE_ERROR(svr->m_em, "%s: recharge: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
                logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_SERVICE_ERROR);
                return logic_op_exec_result_false;
            }
        }
    }
}

static int payment_svr_op_recharge_commit_check_adapter_complete(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record)
{
    logic_require_t require;
    
    if (!logic_stack_node_have_waiting_require(stack)) {
        CPE_INFO(
            svr->m_em, "%s: recharge commit: after op: dump record: %s!",
            payment_svr_name(svr),
            dr_json_dump_inline(
                gd_app_tmp_buffer(svr->m_app),
                record, sizeof(*record), svr->m_meta_recharge_record));
                            
        /*充值的操作已经完成，按照当前状态创建响应 */
        if (payment_svr_op_recharge_commit_build_result(ctx, svr, record) != 0) return -1;
                        
        /*启动充值记录更新 */
        require = logic_require_create(stack, "record_update");
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: commit: create record_update require fail!", payment_svr_name(svr));
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        record->commit_time = payment_svr_cur_time(svr);
        record->version++;
        payment_svr_db_recharge_send_update_state(svr, require, record);
    }

    return 0;
}

static int payment_svr_op_recharge_commit_build_result(logic_context_t ctx, payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record) {
    logic_data_t res_data;
    SVR_PAYMENT_RES_RECHARGE_COMMIT * res;

    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_recharge_commit, sizeof(SVR_PAYMENT_RES_RECHARGE_COMMIT));
    if (res_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: build result: alloc buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }
    
    res = logic_data_data(res_data);

    res->state = record->state;
    res->error = record->error;
    cpe_str_dup(res->error_msg, sizeof(res->error_msg), record->error_msg);
    res->balance.count = 0;

    /* if (payment_svr_db_build_balance(svr, bag_info, require, &res->balance) != 0) { */
    /*     logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
    /*     return logic_op_exec_result_false; */
    /* } */
    
    return 0;
}

static PAYMENT_BILL_DATA *
payment_svr_op_recharge_build_bill(logic_context_t ctx, payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record, BAG_INFO const * bag_info) {
    logic_data_t bill_data;
    PAYMENT_BILL_DATA * bill;
    uint8_t i;
    
    /*构造bill_data */
    bill_data = logic_context_data_get_or_create(ctx, svr->m_meta_bill_data, sizeof(*bill));
    if (bill_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: alloc bill_data fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return NULL;
    }
    bill = logic_data_data(bill_data);

    bill->way = payment_bill_way_in;

    for(i = 0; i < record->money_count; ++i) {
        uint8_t money_type = record->moneies[i].type;
        if (!payment_svr_meta_bag_info_support_money_type(bag_info, money_type)) continue;

        bill->money.datas[bill->money.count].type = money_type;
        bill->money.datas[bill->money.count].count = record->moneies[i].count;
        bill->money.count++;
    }
    snprintf(bill->recharge_way_info, sizeof(bill->recharge_way_info), "trade_id=%s", record->_id);

    return bill;
}

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_record_query(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    int r;
    logic_data_t record_list_data;
    PAYMENT_RECHARGE_RECORD_LIST const * record_list;
    logic_data_t record_data;
    PAYMENT_RECHARGE_RECORD * record;
    payment_svr_adapter_t adapter;
    
    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    /*构造record */
    record_list_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_recharge_record_list));
    if (record_list_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: on query result: find record list fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    record_list = logic_data_data(record_list_data);

    if (record_list->count == 0) {
        CPE_ERROR(
            svr->m_em, "%s: recharge commit: on query result: record not exist, trade %s not exist!",
            payment_svr_name(svr), req->trade_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_NOT_EXIST);
        return logic_op_exec_result_false;
    }

    record_data = logic_context_data_get_or_create(ctx, svr->m_meta_recharge_record, sizeof(PAYMENT_RECHARGE_RECORD));
    if (record_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: on query result: create record buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    record = logic_data_data(record_data);
    memcpy(record, &record_list->records[0], sizeof(PAYMENT_RECHARGE_RECORD));

    CPE_INFO(
        svr->m_em, "%s: recharge commit: before op: dump record: %s!",
        payment_svr_name(svr), dr_json_dump_inline(gd_app_tmp_buffer(svr->m_app), record, sizeof(*record), svr->m_meta_recharge_record));
    
    if(record->state != PAYMENT_RECHARGE_INPROCESS) {
        if (payment_svr_op_recharge_commit_build_result(ctx, svr, record) != 0) {
            return logic_op_exec_result_false;
        }
        else {
            return logic_op_exec_result_true;
        }
    }
    else {
        /*启动外部验证 */
        adapter = payment_svr_adapter_find_by_type_id(svr, record->service);
        if (adapter == NULL) {
            CPE_ERROR(svr->m_em, "%s: recharge commit: service %d is unknown!", payment_svr_name(svr), record->service);
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_SERVICE_ERROR);
            return logic_op_exec_result_false;
        }

        assert(adapter->m_type->m_send);
        r = adapter->m_type->m_send(ctx, stack, adapter, record, req);
        if (r < 0) {
            logic_context_errno_set(ctx, r);
            return logic_op_exec_result_false;
        }

        if (payment_svr_op_recharge_commit_check_adapter_complete(ctx, stack, svr, record) != 0) {
            return logic_op_exec_result_false;
        }

        return logic_op_exec_result_true;
    }
}

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_record_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record)
{
    BAG_INFO const * bag_info;
    PAYMENT_BILL_DATA * bill_data;
    logic_require_t update_require;
    int r;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    if (record->state != PAYMENT_RECHARGE_SUCCESS) {
        /*充值状态还没有结果，则无需更新金币 */
        return logic_op_exec_result_true;
    }
    
    /*充值已经有明确结果了，需要更新充值记录、钱包数据，并记录账单 */
    bag_info = payment_svr_meta_bag_info_find(svr, record->bag_id);
    assert(bag_info);
    
    bill_data = payment_svr_op_recharge_build_bill(ctx, svr, record, bag_info);
    if (bill_data == NULL) {
        return logic_op_exec_result_false;
    }

    /*发送更新请求 */
    update_require = logic_require_create(stack, "data_update");
    if (update_require == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: create logic data_update require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (payment_svr_db_send_add_money(svr, bag_info, update_require, record->user_id, &bill_data->money) != 0) {
        logic_require_free(update_require);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_data_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record)
{
    PAYMENT_BILL_DATA * bill_data;
    mongo_cli_result_t update_result;
    BAG_INFO const * bag_info;
    SVR_PAYMENT_RES_RECHARGE_COMMIT * res;
    int r;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    update_result = mongo_cli_result_find(require);
    if (update_result == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge commit: no db update result!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    bill_data = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_bill_data)));
    assert(bill_data);

    bag_info = payment_svr_meta_bag_info_find(svr, record->bag_id);
    assert(bag_info);

    res = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_recharge_commit)));
    assert(res);
    
    if (mongo_cli_result_n(update_result) == 1) { /*老用户更新成功，查询回数据 */
        payment_svr_db_add_bill(svr, bag_info, record->user_id, bill_data, &res->balance);
        return logic_op_exec_result_true;
    }
    else if (mongo_cli_result_n(update_result) == 0) { /*新用户，需要插入记录 */
        logic_require_t insert_require;

        insert_require = logic_require_create(stack, "data_insert");
        if (insert_require == NULL) {
            CPE_ERROR(svr->m_em, "%s: recharge commit: create logic require for insert fail!", payment_svr_name(svr));
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        if (payment_svr_db_send_init_money(svr, bag_info, insert_require, record->user_id, &bill_data->money) != 0) {
            logic_require_free(insert_require);
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        return logic_op_exec_result_true;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: recharge: on_db_update:: update %d records !!!",
            payment_svr_name(svr), mongo_cli_result_n(update_result));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

static logic_op_exec_result_t
payment_svr_op_recharge_commit_on_data_insert(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD const * record)
{
    BAG_INFO const * bag_info;
    PAYMENT_BILL_DATA * bill_data;
    SVR_PAYMENT_RES_RECHARGE_COMMIT * res;
    int r;
    uint8_t i;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    bag_info = payment_svr_meta_bag_info_find(svr, record->bag_id);
    assert(bag_info);
    
    bill_data = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_bill_data)));
    assert(bill_data);

    res = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_recharge_commit)));
    assert(res);
    
    res->balance.count = bag_info->money_type_count;
    for(i = 0; i < bag_info->money_type_count; ++i) {
        res->balance.datas[i].type = PAYMENT_MONEY_TYPE_MIN + i;
        res->balance.datas[i].count = 
            payment_svr_get_count_by_type(&bill_data->money, res->balance.datas[i].type, 0);
    }

    payment_svr_db_add_bill(svr, bag_info, record->user_id, bill_data, &res->balance);

    return logic_op_exec_result_true;
}

