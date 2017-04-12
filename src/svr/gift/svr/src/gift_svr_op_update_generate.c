#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/time_utils.h"
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
#include "gift_svr_generate_record.h"

logic_op_exec_result_t
gift_svr_op_update_generate_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    gift_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_GIFT_REQ_UPDATE_GENERATE const * req;
    void const * generate_record;

    if (svr->m_init_state != gift_svr_init_success) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: update: svr not init, retry later!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    req_data = logic_context_data_find(ctx, "svr_gift_req_update_generate");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: update: read record: find req fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    /*检查参数 */
    generate_record = gift_svr_record_find(svr, req->generate_id);
    if (generate_record == NULL) {
        CPE_ERROR(svr->m_em, "%s: update: generator %d not exist!", gift_svr_name(svr), req->generate_id);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (req->expire_time) {
        if (req->expire_time < req->begin_time || req->expire_time < gift_svr_cur_time(svr)) {
            char buf1[64]; char buf2[64];

            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: update: time scope %s ~ %s error!",
                gift_svr_name(svr),
                time_to_str((time_t)req->begin_time, buf1, sizeof(buf1)),
                time_to_str((time_t)req->expire_time, buf2, sizeof(buf2)));

            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
    }
    
    /*将生成信息插入数据库 */
    require = logic_require_create(stack, "generate_update");
    if (require == NULL) {
        CPE_ERROR(svr->m_em, "%s: update: create require fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    gift_svr_db_send_generate_update_duration(svr, require, req->generate_id, req->expire_time, req->expire_time);

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_update_generate_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;
    SVR_GIFT_REQ_UPDATE_GENERATE const * req;
    void * generate_record;
    SVR_GIFT_GENERATE_RECORD * generate_record_common;

    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) {
        return logic_op_exec_result_false;
    }

    req = logic_data_data(logic_context_data_find(ctx, "svr_gift_req_update_generate"));

    generate_record = gift_svr_record_find(svr, req->generate_id);
    if (generate_record == NULL) {
        CPE_ERROR(svr->m_em, "%s: update: (recv) generator %d not exist!", gift_svr_name(svr), req->generate_id);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    generate_record_common = (SVR_GIFT_GENERATE_RECORD *)generate_record;

    generate_record_common->begin_time = req->begin_time;
    generate_record_common->expire_time = req->expire_time;
    svr->m_next_expire_time = 0;
    
    return logic_op_exec_result_true;
}
