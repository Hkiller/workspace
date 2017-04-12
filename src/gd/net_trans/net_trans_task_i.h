#ifndef GD_NET_TRANS_TASK_I_H
#define GD_NET_TRANS_TASK_I_H
#include "gd/net_trans/net_trans_task.h"
#include "net_trans_group_i.h"

struct net_trans_task {
    net_trans_group_t m_group;
    uint32_t m_id;
    size_t m_capacity;
    net_trans_task_state_t m_state;
    net_trans_task_result_t m_result;
    net_trans_errno_t m_errno;
    uint8_t m_in_callback;
    uint8_t m_is_free;
    CURL * m_handler;
    curl_socket_t m_sockfd;
    int m_evset;
    struct ev_io m_watch;
    net_trans_task_commit_op_t m_commit_op;
    void * m_commit_ctx;
    void (*m_commit_ctx_free)(void *);
    net_trans_task_write_op_t m_write_op;
    void * m_write_ctx;
    void (*m_write_ctx_free)(void *);
    net_trans_task_progress_op_t m_progress_op;
    void * m_progress_ctx;
    void (*m_progress_ctx_free)(void *);
    struct curl_slist * m_header;
    
    struct mem_buffer m_buffer;
    TAILQ_ENTRY(net_trans_task) m_next_for_group;
    struct cpe_hash_entry m_hh_for_mgr;
};

/*task ops*/
int net_trans_task_set_done(net_trans_task_t task, net_trans_task_result_t result, int err);

uint32_t net_trans_task_hash(net_trans_task_t task);
int net_trans_task_eq(net_trans_task_t l, net_trans_task_t r);

#endif


