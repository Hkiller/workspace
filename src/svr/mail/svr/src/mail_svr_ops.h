#ifndef SVR_MAIL_SVR_OPS_H
#define SVR_MAIL_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "mail_svr_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "protocol/svr/mail/svr_mail_pro.h"

/*operations of mail_svr */
mail_svr_t
mail_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mongo_id_generator_t id_generator,
    mem_allocrator_t alloc,
    error_monitor_t em);

void mail_svr_free(mail_svr_t svr);

mail_svr_t mail_svr_find(gd_app_context_t app, cpe_hash_string_t name);
mail_svr_t mail_svr_find_nc(gd_app_context_t app, const char * name);
const char * mail_svr_name(mail_svr_t svr);

uint32_t mail_svr_cur_time(mail_svr_t svr);

/*db ops*/
int mail_svr_db_send_query(
    mail_svr_t svr, logic_require_t require,
    SVR_MAIL_QUERY_CONDITION const * condition, uint32_t after_time, uint32_t require_count,
    LPDRMETA result_meta);
int mail_svr_db_send_query_global(mail_svr_t svr, logic_require_t require, uint32_t after_time);
int mail_svr_db_send_insert(mail_svr_t svr, logic_require_t require, void const * record);
int mail_svr_db_send_global_insert(mail_svr_t svr, logic_require_t require, void const * record);
int mail_svr_db_send_remove(mail_svr_t svr, logic_require_t require, uint64_t mail_id, uint64_t receiver_gid);
int mail_svr_db_send_update(
    mail_svr_t svr, logic_require_t require, uint64_t mail_id, uint64_t receiver_gid,
    SVR_MAIL_OP const * ops, uint8_t op_count);

/*ops*/
logic_op_exec_result_t
mail_svr_op_query_full_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_query_full_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_query_basic_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_query_basic_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_query_detail_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_query_detail_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_send_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_send_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_send_global_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_send_global_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_query_global_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_query_global_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_remove_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_remove_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
mail_svr_op_update_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
mail_svr_op_update_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

#endif
