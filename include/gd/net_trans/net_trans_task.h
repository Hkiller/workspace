#ifndef GD_DR_STORE_H
#define GD_DR_STORE_H
#include "cpe/utils/buffer.h"
#include "net_trans_types.h"

#ifdef __cplusplus
extern "C" {
#endif

net_trans_task_t net_trans_task_create(net_trans_group_t group, size_t capacity);
void net_trans_task_free(net_trans_task_t task);

uint32_t net_trans_task_id(net_trans_task_t task);
net_trans_group_t net_trans_task_group(net_trans_task_t task);
net_trans_manage_t net_trans_task_manage(net_trans_task_t task);

net_trans_task_state_t net_trans_task_state(net_trans_task_t task);
net_trans_task_result_t net_trans_task_result(net_trans_task_t task);
net_trans_errno_t net_trans_task_errno(net_trans_task_t task);

void * net_trans_task_data(net_trans_task_t task);
size_t net_trans_task_data_capacity(net_trans_task_t task);

mem_buffer_t net_trans_task_buffer(net_trans_task_t task);
const char * net_trans_task_buffer_to_string(net_trans_task_t task);

int net_trans_task_start(net_trans_task_t task);

int net_trans_task_restart(net_trans_task_t task);
    
int net_trans_task_set_post_to(net_trans_task_t task, const char * uri, const char * data, int data_len);
int net_trans_task_set_get(net_trans_task_t task, const char * uri);

void net_trans_task_set_debug(net_trans_task_t task, uint8_t is_debug);

void net_trans_task_set_commit_op(net_trans_task_t task, net_trans_task_commit_op_t op, void * ctx, void (*ctx_free)(void *));
void net_trans_task_set_progress_op(net_trans_task_t task, net_trans_task_progress_op_t op, void * ctx, void (*ctx_free)(void *));
void net_trans_task_set_write_op(net_trans_task_t task, net_trans_task_write_op_t op, void * ctx, void (*ctx_free)(void *));
    
int net_trans_task_set_ssl_cainfo(net_trans_task_t task, const char * ca_file);
int net_trans_task_set_skip_data(net_trans_task_t task, ssize_t skip_length);
int net_trans_task_set_timeout(net_trans_task_t task, uint64_t timeout_ms);

int net_trans_task_set_useragent(net_trans_task_t task, const char * agent);
int net_trans_task_append_header(net_trans_task_t task, const char * header_one);
    
const char * net_trans_task_state_str(net_trans_task_state_t state);
const char * net_trans_task_result_str(net_trans_task_result_t result);

#ifdef __cplusplus
}
#endif

#endif
