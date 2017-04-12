#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "mongo_connection_i.h"
#include "mongo_manip_i.h"

static uint32_t mongo_connection_fsm_check_is_master_on_response(mongo_connection_t connection, mongo_pkg_t pkg);

static void mongo_connection_fsm_check_is_master_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;
    mongo_pkg_t pkg_buf;

    if (connection->m_server->m_has_is_master) {
        mongo_connection_fsm_apply_recv_pkg(connection, NULL);
        return;
    }
    
    assert(connection->m_fd != -1);

    if (mongo_connection_start_state_timer(connection, driver->m_op_timeout_ms) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: start check-is-master timer fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }

    pkg_buf = mongo_pkg_build_check_is_master(driver);
    if (pkg_buf == NULL || mongo_connection_send(connection, pkg_buf) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: check is master fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_stop);
        return;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: check is master: send cmd success!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
    }
}

static void mongo_connection_fsm_check_is_master_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);

    mongo_connection_stop_state_timer(connection);
    mongo_connection_stop_watch(connection);
}

static uint32_t mongo_connection_fsm_check_is_master_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_connection_fsm_evt * evt = input_evt;
    mongo_connection_t connection = fsm_machine_context(fsm);

    switch(evt->m_type) {
    case mongo_connection_fsm_evt_wb_update:
        mongo_connection_start_watch(connection);
        return FSM_KEEP_STATE;
    case mongo_connection_fsm_evt_recv_pkg:
        if (connection->m_server->m_has_is_master) return mongo_connection_state_authenticate;
        return mongo_connection_fsm_check_is_master_on_response(connection, evt->m_pkg);
    case mongo_connection_fsm_evt_stop:
    case mongo_connection_fsm_evt_disconnected:
    case mongo_connection_fsm_evt_timeout:
        return mongo_connection_state_disable;
    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_connection_fsm_create_check_is_master(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "check_is_master", mongo_connection_state_check_is_master);
    if (s == NULL) {
        CPE_ERROR(em, "%s: check-is-master: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_connection_fsm_check_is_master_enter, mongo_connection_fsm_check_is_master_leave);

    if (fsm_def_state_add_transition(s, mongo_connection_fsm_check_is_master_trans) != 0) {
        CPE_ERROR(em, "%s: check-is-master: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static uint32_t mongo_connection_fsm_check_is_master_on_response(mongo_connection_t connection, mongo_pkg_t pkg) {
    if (mongo_server_update_info(connection->m_server, pkg) != 0) {
        return mongo_connection_state_disable;
    }

    switch(connection->m_server->m_mode) {
    case mongo_server_runing_mode_rs_arbiter:
    case mongo_server_runing_mode_rs_other:
    case mongo_server_runing_mode_rs_ghost:
        mongo_server_offline(connection->m_server);
        return mongo_connection_state_disable;
    default:
        return mongo_connection_state_authenticate;
    }
}
