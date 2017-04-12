#ifndef SVR_SET_SVR_ROUTER_CONN_H
#define SVR_SET_SVR_ROUTER_CONN_H
#include "set_svr_set.h"

enum set_svr_set_conn_state {
    set_svr_set_conn_state_connecting
    , set_svr_set_conn_state_established
    , set_svr_set_conn_state_registing
    , set_svr_set_conn_state_accepting
};

struct set_svr_set_conn {
    set_svr_t m_svr;
    set_svr_set_t m_set;

    uint32_t m_conn_id;
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_tb;

    int m_fd;
    struct ev_io m_watcher;

    TAILQ_ENTRY(set_svr_set_conn) m_next;
};

/*operations of set_svr_set_conn*/
set_svr_set_conn_t set_svr_set_conn_create(set_svr_t svr, set_svr_set_t set, int fd);
void set_svr_set_conn_free(set_svr_set_conn_t conn);
void set_svr_set_conn_free_all(set_svr_t svr);

int set_svr_set_conn_send(set_svr_set_conn_t conn, uint16_t to_svr_id, dp_req_t body, dp_req_t head, dp_req_t carry, size_t * write_size);

void set_svr_set_conn_link_node_r(set_svr_set_conn_t conn, ringbuffer_block_t blk);
void set_svr_set_conn_link_node_w_head(set_svr_set_conn_t conn, ringbuffer_block_t blk);
void set_svr_set_conn_link_node_w_tail(set_svr_set_conn_t conn, ringbuffer_block_t blk);
int set_svr_set_conn_read_from_net(set_svr_set_conn_t conn, size_t require_size);
int set_svr_set_conn_write_to_net(set_svr_set_conn_t conn);

int set_svr_set_conn_r_buf(set_svr_set_conn_t conn, size_t require_size, void * * buf);
void set_svr_set_conn_r_erase(set_svr_set_conn_t conn, size_t size);

#endif
