#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "usf/logic/logic_data.h"
#include "gift_svr_ops.h"
#include "gift_svr_db_ops_generate.h"
#include "gift_svr_db_ops_use.h"
#include "gift_svr_generate_record.h"

logic_op_exec_result_t
gift_svr_op_expire_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;
    struct aom_obj_it obj_it;
    SVR_GIFT_GENERATE_RECORD * record_common;

    aom_objs(svr->m_generate_record_mgr, &obj_it);
    while((record_common = aom_obj_it_next(&obj_it))) {
        if (record_common->expire_time < gift_svr_cur_time(svr)) break;
    }

    if (record_common == NULL) {
        CPE_INFO(svr->m_em, "%s: expire: no expire generate", gift_svr_name(svr));
        svr->m_expire_op_in_process = 0;
        svr->m_next_expire_time = 0;
        return logic_op_exec_result_true;
    }
    else {
        CPE_INFO(
            svr->m_em, "%s: expire: found generate: %s",
            gift_svr_name(svr), gift_svr_record_dump(svr, record_common));
    }

    /*cdkey资源相对紧缺，需要回首 */
    gift_svr_db_send_use_remove_by_generate_id(svr, NULL, record_common->_id);

    /*生成信息从缓冲中清除，生成记录保留 */
    aom_obj_hash_table_remove_by_ins(svr->m_generate_record_hash, record_common);

    svr->m_next_expire_time = 0;
    svr->m_expire_op_in_process = 0;
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_expire_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    return logic_op_exec_result_true;
}
