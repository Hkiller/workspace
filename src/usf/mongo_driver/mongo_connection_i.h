#ifndef USF_MONGO_DRIVER_CONNECTION_I_H
#define USF_MONGO_DRIVER_CONNECTION_I_H
#include "mongo_server_i.h"

struct mongo_connection {
    mongo_server_t m_server;
    TAILQ_ENTRY(mongo_connection) m_next_for_server;
    uint32_t m_id;
    
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    uint8_t m_sub_step;
    
    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;

    uint16_t m_to_send_pkg_count;
    uint32_t m_sending_pkg_op;
    uint8_t m_runing_pkg_begin;
    uint8_t m_runing_pkg_end;
    uint64_t m_runing_pkg_send_times[16];
    
    void * m_addition;
    void (*m_addition_cleanup)(mongo_driver_t driver, void *);
};

enum mongo_connection_state {
    mongo_connection_state_disable
    , mongo_connection_state_connecting
    , mongo_connection_state_check_is_master
    , mongo_connection_state_authenticate
    , mongo_connection_state_check_readable
    , mongo_connection_state_connected
};

enum mongo_connection_fsm_evt_type {
    mongo_connection_fsm_evt_start
    , mongo_connection_fsm_evt_stop
    , mongo_connection_fsm_evt_connected
    , mongo_connection_fsm_evt_disconnected
    , mongo_connection_fsm_evt_timeout
    , mongo_connection_fsm_evt_wb_update
    , mongo_connection_fsm_evt_recv_pkg
};

struct mongo_connection_fsm_evt {
    enum mongo_connection_fsm_evt_type m_type;
    mongo_pkg_t m_pkg;
};

mongo_connection_t mongo_connection_create(mongo_server_t server);
void mongo_connection_free(mongo_connection_t connection);
void mongo_connection_free_all(mongo_server_t server);

fsm_def_machine_t mongo_connection_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

void mongo_connection_disconnect(mongo_connection_t connection);
mongo_connection_t mongo_connection_find_by_fd(mongo_driver_t driver, int fd);
void mongo_connection_link_node_r(mongo_connection_t server, ringbuffer_block_t blk);
void mongo_connection_link_node_w(mongo_connection_t server, ringbuffer_block_t blk);
void mongo_connection_stop_watch(mongo_connection_t connection);
void mongo_connection_start_watch(mongo_connection_t server);

void mongo_connection_fsm_apply_evt(mongo_connection_t connection, enum mongo_connection_fsm_evt_type type);
void mongo_connection_fsm_apply_recv_pkg(mongo_connection_t connection, mongo_pkg_t pkg);
int mongo_connection_start_state_timer(mongo_connection_t connection, tl_time_span_t span);
void mongo_connection_stop_state_timer(mongo_connection_t connection);

int mongo_connection_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_connection_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_connection_fsm_create_authenticate(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_connection_fsm_create_check_is_master(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_connection_fsm_create_check_readable(fsm_def_machine_t fsm_def, error_monitor_t em);
int mongo_connection_fsm_create_connected(fsm_def_machine_t fsm_def, error_monitor_t em);

int mongo_connection_alloc(ringbuffer_block_t * result, mongo_driver_t driver, mongo_connection_t server, size_t size);
void mongo_connection_rw_cb(EV_P_ ev_io *w, int revents);

int mongo_connection_send(mongo_connection_t connection, mongo_pkg_t pkg);

uint8_t mongo_connection_runing_pkg_count(mongo_connection_t connection);
uint8_t mongo_connection_runing_pkg_full(mongo_connection_t connection);
void mongo_connection_runing_pkg_push(mongo_connection_t connection);
void mongo_connection_runing_pkg_pop(mongo_connection_t connection);

void mongo_connection_clear_addition(mongo_connection_t connection);

#endif
