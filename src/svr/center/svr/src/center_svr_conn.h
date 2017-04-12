#ifndef SVR_CENTER_SVR_CONN_H
#define SVR_CENTER_SVR_CONN_H
#include "center_svr.h"

struct center_svr_conn {
    center_svr_t m_svr;
    center_svr_set_proxy_t m_set;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_tb;

    struct cpe_hash_entry m_hh;
};

center_svr_conn_t center_svr_conn_create(center_svr_t svr, int fd);
void center_svr_conn_free(center_svr_conn_t conn);
void center_svr_conn_free_all(center_svr_t svr);
int center_svr_conn_send(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);

void center_svr_conn_start_watch(center_svr_conn_t conn);
void center_svr_conn_link_node_r(center_svr_conn_t conn, ringbuffer_block_t blk);
void center_svr_conn_link_node_w(center_svr_conn_t conn, ringbuffer_block_t blk);
int center_svr_conn_alloc(ringbuffer_block_t * result, center_svr_t svr, center_svr_conn_t conn, size_t size);

uint32_t center_svr_conn_hash(center_svr_conn_t conn);
int center_svr_conn_eq(center_svr_conn_t l, center_svr_conn_t r);

#endif
