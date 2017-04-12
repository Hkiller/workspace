#ifndef SVR_SET_SVR_TYPES_CENTER_H
#define SVR_SET_SVR_TYPES_CENTER_H
#include "set_svr.h"

enum set_svr_center_state {
    set_svr_center_state_disable
    , set_svr_center_state_disconnected
    , set_svr_center_state_connecting
    , set_svr_center_state_join
    , set_svr_center_state_idle
};

struct set_svr_center {
    set_svr_t m_svr;

    fsm_def_machine_t m_fsm_def;
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    uint32_t m_read_block_size;
    uint32_t m_max_pkg_size;
    uint32_t m_conn_id;

    uint32_t m_reconnect_span_ms;
    uint32_t m_update_span_s;
    char m_ip[16];
    uint16_t m_port;

    LPDRMETA m_pkg_meta;

    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_tb;

    int m_fd;
    struct ev_io m_watcher;

    struct mem_buffer m_outgoing_pkg_buf;
};

set_svr_center_t set_svr_center_create(set_svr_t agent);
void set_svr_center_free(set_svr_center_t center);

int set_svr_center_set_svr(set_svr_center_t center, const char * ip, short port);
int set_svr_center_set_reconnect_span_ms(set_svr_center_t center, uint32_t span_ms);

int set_svr_center_send(set_svr_center_t center, SVR_CENTER_PKG * pkg, size_t pkg_size);
void set_svr_center_disconnect(set_svr_center_t center);

int set_svr_center_start_state_timer(set_svr_center_t center, tl_time_span_t span);
void set_svr_center_stop_state_timer(set_svr_center_t center);

void set_svr_center_link_node_r(set_svr_center_t center, ringbuffer_block_t blk);
void set_svr_center_link_node_w(set_svr_center_t center, ringbuffer_block_t blk);
void set_svr_center_start_watch(set_svr_center_t center);

void set_svr_center_rw_cb(EV_P_ ev_io *w, int revents);

SVR_CENTER_PKG * set_svr_center_get_pkg_buff(set_svr_center_t center, size_t capacity);

#endif
