#ifndef SVR_FRIEND_SVR_OPS_H
#define SVR_FRIEND_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "friend_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/friend/svr_friend_pro.h"

/*operations of friend_svr */
friend_svr_t
friend_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em);

void friend_svr_free(friend_svr_t svr);

friend_svr_t friend_svr_find(gd_app_context_t app, cpe_hash_string_t name);
friend_svr_t friend_svr_find_nc(gd_app_context_t app, const char * name);
const char * friend_svr_name(friend_svr_t svr);

/*record ops*/
uint64_t friend_svr_record_uid(friend_svr_t svr, void * record);
void friend_svr_record_set_uid(friend_svr_t svr, uint64_t uid, void * record);
uint64_t friend_svr_record_fuid(friend_svr_t svr, void * record);
void friend_svr_record_set_fuid(friend_svr_t svr, uint64_t fuid, void * record);
uint8_t friend_svr_record_state(friend_svr_t svr, void * record);
void friend_svr_record_set_state(friend_svr_t svr, uint8_t state, void * record);

int friend_svr_record_build_id(friend_svr_t svr, void * record);

/*db ops*/
int friend_svr_db_send_query(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t user_id, uint8_t state_count, uint8_t * states);
int friend_svr_db_send_query_data(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t user_id, uint16_t friend_count, uint64_t * friends);
int friend_svr_db_send_query_one(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t user_id, uint64_t friend_uid);
int friend_svr_db_send_insert(friend_svr_t svr, logic_stack_node_t stack, const char * req_name , void const * record);
int friend_svr_db_send_remove(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t uid, uint64_t fuid);
int friend_svr_db_send_update_state(
    friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t uid, uint64_t fuid, uint8_t from_state, uint8_t to_state);

/*ops*/
logic_op_exec_result_t
friend_svr_op_query_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
friend_svr_op_query_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
friend_svr_op_query_data_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
friend_svr_op_query_data_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
friend_svr_op_add_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
friend_svr_op_add_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
friend_svr_op_remove_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
friend_svr_op_remove_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
friend_svr_op_sync_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
friend_svr_op_sync_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
friend_svr_op_ack_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
friend_svr_op_ack_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

#endif
