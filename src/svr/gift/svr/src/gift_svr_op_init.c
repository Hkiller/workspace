#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "gift_svr_ops.h"
#include "gift_svr_db_ops_generate.h"
#include "gift_svr_generate_record.h"

logic_op_exec_result_t
gift_svr_op_init_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;

    if (svr->m_init_state != gift_svr_init_not_init) return logic_op_exec_result_true;

    if (aom_obj_mgr_allocked_obj_count(svr->m_generate_record_mgr) != 0) {
        /*本地缓冲不为空，则从本地缓冲中恢复 */
        struct aom_obj_it obj_it;
        SVR_GIFT_GENERATE_RECORD const * record_common;

        aom_objs(svr->m_generate_record_mgr, &obj_it);
        while((record_common = aom_obj_it_next(&obj_it))) {
            if (record_common->_id > svr->m_max_generate_id) {
                svr->m_max_generate_id = record_common->_id;
            }

            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: init: found generate: %s",
                    gift_svr_name(svr), gift_svr_record_dump(svr, record_common));
            }
        }
        
        CPE_INFO(
            svr->m_em, "%s: init: reuse cache data, max_generate_id=%d",
            gift_svr_name(svr), svr->m_max_generate_id);
        
        svr->m_init_state = gift_svr_init_success;
        
        return logic_op_exec_result_true;
    }
    else {
        /*本地缓冲为空，发送请求从数据库中查询 */
        logic_require_t require;
        
        require = logic_require_create(stack, "db_query");
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: init: create require fail!", gift_svr_name(svr));
            return logic_op_exec_result_false;
        }
    
        if (gift_svr_db_send_generate_record_query(svr, require) != 0) {
            CPE_ERROR(svr->m_em, "%s: init: send query request fail!", gift_svr_name(svr));
            logic_require_free(require);
            return logic_op_exec_result_false;
        }

        svr->m_init_state = gift_svr_init_loading;
    }
    
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_init_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;
    logic_data_t query_result_data;
    uint8_t * query_result;
    uint32_t record_count;
    uint32_t i;
    uint32_t cur_time;
    
    assert(svr->m_init_state == gift_svr_init_loading);
    
    if (logic_require_state(require) != logic_require_state_done) {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: init: db request error, errno=%d!",
            gift_svr_name(svr), logic_require_error(require));
        svr->m_init_state = gift_svr_init_not_init;
        return logic_op_exec_result_false;
    }

    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_generate_record_list_meta));
    if (query_result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init: query result %s not exist!",
            gift_svr_name(svr), dr_meta_name(svr->m_generate_record_list_meta));
        svr->m_init_state = gift_svr_init_not_init;
        return logic_op_exec_result_false;
    }
    query_result = logic_data_data(query_result_data);

    if (dr_entry_try_read_uint32(
            &record_count,
            query_result + dr_entry_data_start_pos(svr->m_generate_record_list_count_entry, 0),
            svr->m_generate_record_list_count_entry,
            svr->m_em) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: init: read record count fail!", gift_svr_name(svr));
        svr->m_init_state = gift_svr_init_not_init;
        return logic_op_exec_result_false;
    }

    cur_time =  gift_svr_cur_time(svr);
    for(i = 0; i < record_count; ++i) {
        uint8_t * query_record;
        SVR_GIFT_GENERATE_RECORD const * record_common;

        query_record = query_result + dr_entry_data_start_pos(svr->m_generate_record_list_data_entry, i);

        record_common = (SVR_GIFT_GENERATE_RECORD const *)query_record;
        if (record_common->_id > svr->m_max_generate_id) {
            svr->m_max_generate_id = record_common->_id;
        }

        if (record_common->expire_time == 0 || record_common->expire_time > cur_time) {
            /*没有过期*/
            if (aom_obj_hash_table_insert(svr->m_generate_record_hash, query_record, NULL) != 0) {
                CPE_ERROR(
                    svr->m_em, "%s: init: insert record %s fail!",
                    gift_svr_name(svr), gift_svr_record_dump(svr, query_record));
                continue;
            }

            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: init: load generate: %s",
                    gift_svr_name(svr), gift_svr_record_dump(svr, query_record));
            }
        }
    }
    
    CPE_INFO(
        svr->m_em, "%s: init: init cache success, generate_record-count=%d, max_generate_id=%d!",
        gift_svr_name(svr), aom_obj_mgr_allocked_obj_count(svr->m_generate_record_mgr),
        svr->m_max_generate_id);

    svr->m_init_state = gift_svr_init_success;
    return logic_op_exec_result_true;
}
