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
#include "cpe/aom/aom_obj_mgr.h"
#include "protocol/svr/gift/svr_gift_pro.h"
#include "protocol/svr/gift/svr_gift_internal.h"
#include "gift_svr_ops.h"
#include "gift_svr_generate_record.h"

logic_op_exec_result_t
gift_svr_op_query_generate_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    gift_svr_t svr = user_data;
    struct aom_obj_it obj_it;
    void * record;
    SVR_GIFT_GENERATE_RECORD * record_common;
    uint32_t result_capacity;
    logic_data_t result_data;
    SVR_GIFT_RES_QUERY_GENERATE * result;
    int rv;
    
    result_capacity = sizeof(SVR_GIFT_RES_QUERY_GENERATE) + sizeof(SVR_GIFT_GENERATE) * aom_obj_mgr_allocked_obj_count(svr->m_generate_record_mgr);
    result_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_query_generate, result_capacity);
    if (result_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: create result data fail, capacity=%d!", gift_svr_name(svr), result_capacity);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    result = logic_data_data(result_data);

    result->generate_count = 0;
    aom_objs(svr->m_generate_record_mgr, &obj_it);
    while((record = aom_obj_it_next(&obj_it))) {
        SVR_GIFT_GENERATE * result_record = &result->generates[result->generate_count++];
        record_common = record;

        result_record->generate_id = record_common->_id;
        result_record->begin_time = record_common->begin_time;
        result_record->expire_time = record_common->expire_time;
        result_record->use_policy = record_common->use_policy;

        rv = dr_pbuf_write(
            result_record->data, sizeof(result_record->data),
            ((char*)record) + svr->m_generate_record_data_start_pos, svr->m_data_size, svr->m_data_meta,
            svr->m_em);
        if (rv < 0) {
            CPE_ERROR(
                svr->m_em, "%s: query generate: generate %d build result: encode data fail, capacity=%d, rv=%d!",
                gift_svr_name(svr), record_common->_id, (int)(sizeof(result_record->data)), rv);
            result_record->data_len = 0;
        }
        else {
            result_record->data_len = rv;
        }
    }        
    
    return logic_op_exec_result_true;
}
