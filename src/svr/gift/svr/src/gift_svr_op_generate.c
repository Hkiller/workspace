#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/time_utils.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "protocol/svr/gift/svr_gift_pro.h"
#include "protocol/svr/gift/svr_gift_internal.h"
#include "gift_svr_ops.h"
#include "gift_svr_db_ops_generate.h"
#include "gift_svr_db_ops_use.h"
#include "gift_svr_generate_record.h"
#include "gift_svr_generator.h"

static logic_op_exec_result_t
gift_svr_op_generate_recv_generate_insert(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, gift_svr_t svr);
static logic_op_exec_result_t
gift_svr_op_generate_recv_use_insert(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, gift_svr_t svr);
static logic_op_exec_result_t
gift_svr_op_generate_recv_check_use_insert_result(
    logic_context_t ctx, gift_svr_t svr, SVR_GIFT_OP_GENERATE_CTX * op_ctx, SVR_GIFT_RES_GENERATE * result);

logic_op_exec_result_t
gift_svr_op_generate_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    gift_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_GIFT_REQ_GENERATE const * req;
    logic_data_t op_ctx_data;
    SVR_GIFT_OP_GENERATE_CTX * op_ctx;
    SVR_GIFT_GENERATE_RECORD * record_common;
    gift_svr_generator_t generator;
    uint32_t result_capacity;
    logic_data_t result_data;
    SVR_GIFT_RES_GENERATE * result;
    int rv;

    req_data = logic_context_data_find(ctx, "svr_gift_req_generate");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: read record: find req fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    /*检查参数 */
    if(req->use_policy != svr_gift_use_once_global && req->use_policy != svr_gift_use_once_per_user) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: unknown use policy %d!", gift_svr_name(svr), req->use_policy);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (req->expire_time) {
        if (req->expire_time < req->begin_time || req->expire_time < gift_svr_cur_time(svr)) {
            char buf1[64]; char buf2[64];

            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: generate: time scope %s ~ %s error!",
                gift_svr_name(svr),
                time_to_str((time_t)req->begin_time, buf1, sizeof(buf1)),
                time_to_str((time_t)req->expire_time, buf2, sizeof(buf2)));

            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
    }

    generator = gift_svr_generator_find(svr, req->format);
    if (generator == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: format %s unknown!", gift_svr_name(svr), req->format);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if(req->generate_count <= 0 || req->generate_count > generator->m_max_generate_count) {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: generate: generate count %d error, should in [0~%d]!",
            gift_svr_name(svr), req->generate_count, generator->m_max_generate_count);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    /*构造批次信息 */
    op_ctx_data =
        logic_context_data_get_or_create(
            ctx, svr->m_meta_op_generate_ctx,
            dr_meta_size(svr->m_meta_op_generate_ctx) + svr->m_generate_record_size);
    if (op_ctx_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: read generate record: create op_generate ctx fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    op_ctx =(SVR_GIFT_OP_GENERATE_CTX*)logic_data_data(op_ctx_data);

    record_common = (SVR_GIFT_GENERATE_RECORD *)op_ctx->record;
    record_common->_id = svr->m_max_generate_id + 1;
    record_common->cdkey_len = generator->m_cdkey_len;
    record_common->region_id = req->region_id;
    record_common->use_policy = req->use_policy;
    record_common->begin_time = req->begin_time;
    record_common->expire_time = req->expire_time;
    record_common->generate_count = req->generate_count;

    if (gift_svr_generator_select_prefix(generator, record_common->prefix) != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: read generate record: select prefix fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    rv = dr_pbuf_read(
        op_ctx->record + svr->m_generate_record_data_start_pos,
        svr->m_generate_record_size - svr->m_generate_record_data_start_pos,
        req->data,
        req->data_len,
        svr->m_data_meta,
        svr->m_em);
    if (rv <= 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: read generate record: decode data error, rv=%d!", gift_svr_name(svr), rv);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    op_ctx->record_len = (uint16_t)rv;

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: generate: record=%s!", gift_svr_name(svr), gift_svr_record_dump(svr, op_ctx->record));
    }

    /*生成cdkey */
    result_capacity = sizeof(SVR_GIFT_RES_GENERATE) + sizeof(SVR_GIFT_USE_BASIC) * record_common->generate_count;

    result_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_generate, result_capacity);
    if (result_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: create result data fail, capacity=%d!", gift_svr_name(svr), result_capacity);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    result = logic_data_data(result_data);

    result->generate_id = record_common->_id;
    result->record_count = req->generate_count;
    if (gift_svr_generator_gen(generator, record_common->prefix, result->records, &result->record_count) != 0) {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: generate: generate cd key fail, require-count=%d!",
            gift_svr_name(svr), record_common->generate_count);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    /*将生成信息插入数据库 */
    require = logic_require_create(stack, "generate_insert");
    if (require == NULL) {
        CPE_ERROR(svr->m_em, "%s: generate: create require fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    gift_svr_db_send_generate_insert(svr, require, op_ctx->record);

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_generate_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;

    if (strcmp(logic_require_name(require), "generate_insert") == 0) {
        return gift_svr_op_generate_recv_generate_insert(ctx, stack, require, svr);
    }
    else if (cpe_str_start_with(logic_require_name(require), "use_insert_")) {
        return gift_svr_op_generate_recv_use_insert(ctx, stack, require, svr);
    }
    else {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: request %s unknown!", gift_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

static logic_op_exec_result_t
gift_svr_op_generate_recv_generate_insert(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, gift_svr_t svr) {
    SVR_GIFT_OP_GENERATE_CTX * op_ctx;
    SVR_GIFT_GENERATE_RECORD * record_common;
    SVR_GIFT_RES_GENERATE * result;
    SVR_GIFT_USE_RECORD db_record;
    uint16_t i;

    op_ctx =(SVR_GIFT_OP_GENERATE_CTX*)logic_data_data(logic_context_data_find(ctx, "svr_gift_op_generate_ctx"));
    record_common = (SVR_GIFT_GENERATE_RECORD *)op_ctx->record;

    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) {
        return logic_op_exec_result_false;
    }

    result = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_generate)));

    assert(op_ctx->success_count == 0);
    assert(op_ctx->fail_count == 0);

    db_record.generate_id = record_common->_id;
    db_record.state = svr_gift_use_state_not_used;

    for(i = 0; i < result->record_count; ++i) {
        char insert_require_name[32];
        logic_require_t insert_require;

        snprintf(insert_require_name, sizeof(insert_require_name), "use_insert_%d", i);
        insert_require = logic_require_create(stack, insert_require_name);
        if (insert_require == NULL) {
            CPE_ERROR(svr->m_em, "%s: generate: create require %s fail!", gift_svr_name(svr), insert_require_name);
            op_ctx->fail_count++;
            continue;
        }

        cpe_str_dup(db_record._id, sizeof(db_record._id), result->records[i].cdkey);

        gift_svr_db_send_use_insert(svr, insert_require, &db_record);
    }

    return gift_svr_op_generate_recv_check_use_insert_result(ctx, svr, op_ctx, result);
}

static logic_op_exec_result_t
gift_svr_op_generate_recv_use_insert(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, gift_svr_t svr) {
    SVR_GIFT_OP_GENERATE_CTX * op_ctx;
    SVR_GIFT_RES_GENERATE * result;
    int index;

    op_ctx =(SVR_GIFT_OP_GENERATE_CTX*)logic_data_data(logic_context_data_find(ctx, "svr_gift_op_generate_ctx"));
    result = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_generate)));

    sscanf(logic_require_name(require), "use_insert_%d", &index);
    assert(index >= 0 && index < result->record_count);

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            CPE_ERROR(
                svr->m_em, "%s: %s: db request error, errno=%d!",
                gift_svr_name(svr), logic_require_name(require), logic_require_error(require));
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: %s: db request state error, state=%s!",
                gift_svr_name(svr), logic_require_name(require), logic_require_state_name(logic_require_state(require)));
        }

        result->records[index].cdkey[0] = 0;
        op_ctx->fail_count++;
    }
    else {
        op_ctx->success_count++;
    }

    return gift_svr_op_generate_recv_check_use_insert_result(ctx, svr, op_ctx, result);
}

static logic_op_exec_result_t
gift_svr_op_generate_recv_check_use_insert_result(logic_context_t ctx, gift_svr_t svr, SVR_GIFT_OP_GENERATE_CTX * op_ctx, SVR_GIFT_RES_GENERATE * result) {
    SVR_GIFT_GENERATE_RECORD * record_common = (SVR_GIFT_GENERATE_RECORD *)op_ctx->record;

    /*不是所有请求都完成，继续等待 */
    if (op_ctx->success_count + op_ctx->fail_count != result->record_count) {
        return logic_op_exec_result_true;
    }

    /*所有请求完成，有数据库错误的， 剔除失败记录 */
    if (op_ctx->fail_count > 0) {
        uint16_t i;

        for(i = 0; i < result->record_count; ++i) {

            if (result->records[i].cdkey[0] == 0) {
                while(result->record_count - 1 > i && result->records[result->record_count - 1].cdkey[0] == 0) {
                    result->record_count--;
                }

                if (result->record_count - 1 > i) {
                    memcpy(result->records + i, result->records + (result->record_count - 1), sizeof(result->records[0]));
                    result->record_count--;
                }
            }
        }
    }

    /*所有请求完成，没有任何记录插入，则认为完整失败，返回错误码 */
    if (op_ctx->success_count == 0) {
        gift_svr_db_send_generate_remove(svr, NULL, record_common->_id);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_DB);
        return logic_op_exec_result_false;
    }

    /*将生成记录插入缓存 */
    if (aom_obj_hash_table_insert(svr->m_generate_record_hash, op_ctx->record, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: generate: insert generate record to db fail!", gift_svr_name(svr));
        gift_svr_db_send_generate_remove(svr, NULL, record_common->_id);
        gift_svr_db_send_use_remove_by_generate_id(svr, NULL, record_common->_id);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    svr->m_max_generate_id++;
    return logic_op_exec_result_true;
}
