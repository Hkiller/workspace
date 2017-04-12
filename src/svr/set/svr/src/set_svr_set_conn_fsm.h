#ifndef SVR_SET_SVR_ROUTER_CONN_FSM_H
#define SVR_SET_SVR_ROUTER_CONN_FSM_H
#include "set_svr_set_conn.h"

enum set_svr_set_conn_fsm_evt_type {
    set_svr_set_conn_fsm_evt_pkg
    , set_svr_set_conn_fsm_evt_timeout
    , set_svr_set_conn_fsm_evt_connected
    , set_svr_set_conn_fsm_evt_disconnected
    , set_svr_set_conn_fsm_evt_accepted
    , set_svr_set_conn_fsm_evt_registed
    , set_svr_set_conn_fsm_evt_wb_update
};

struct set_svr_set_conn_fsm_evt {
    enum set_svr_set_conn_fsm_evt_type m_type;
    void * m_pkg;
};

fsm_def_machine_t
set_svr_set_conn_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

int set_svr_set_conn_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_set_conn_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_set_conn_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_set_conn_fsm_create_established(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_set_conn_fsm_create_accepting(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_set_conn_fsm_create_registing(fsm_def_machine_t fsm_def, error_monitor_t em);

void set_svr_set_conn_set_set(set_svr_set_conn_t conn, set_svr_set_t set);

void set_svr_set_conn_apply_evt(set_svr_set_conn_t set_conn, enum set_svr_set_conn_fsm_evt_type type);
void set_svr_set_conn_apply_pkg(set_svr_set_conn_t set_conn, SVR_CENTER_PKG * pkg);

int set_svr_set_conn_start_state_timer(set_svr_set_conn_t set_conn, tl_time_span_t span);
void set_svr_set_conn_stop_state_timer(set_svr_set_conn_t set_conn);

#endif
