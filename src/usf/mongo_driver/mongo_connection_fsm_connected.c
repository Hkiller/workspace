#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"

static void mongo_connection_fsm_connected_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);
    connection->m_server->m_active_connection_count++;
}

static void mongo_connection_fsm_connected_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);

    assert(connection->m_server->m_active_connection_count > 0);
    connection->m_server->m_active_connection_count--;
    
    mongo_connection_stop_watch(connection);
}

static uint32_t mongo_connection_fsm_connected_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_connection_fsm_evt * evt = input_evt;
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;

    switch(evt->m_type) {
    case mongo_connection_fsm_evt_stop:
        return mongo_connection_state_disable;
    case mongo_connection_fsm_evt_disconnected:
        return mongo_connection_state_disable;
    case mongo_connection_fsm_evt_wb_update:
        mongo_connection_start_watch(connection);
        return FSM_KEEP_STATE;
    case mongo_connection_fsm_evt_recv_pkg: {
        if (driver->m_incoming_send_to) {
            dp_req_t dp_req = mongo_pkg_to_dp_req(evt->m_pkg);
            if (dp_dispatch_by_string(driver->m_incoming_send_to, dp_req_mgr(dp_req), dp_req, driver->m_em) != 0) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s %d: on read: dispatch to %s fail!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, cpe_hs_data(driver->m_incoming_send_to));
            }
        }
        else {
            if (driver->m_debug >= 2) {
                CPE_INFO(
                    driver->m_em, "%s: server %s %d: on read: no dispatch info, skip!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
            }
        }
        return FSM_KEEP_STATE;
    }
    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_connection_fsm_create_connected(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "connected", mongo_connection_state_connected);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_master: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_connection_fsm_connected_enter, mongo_connection_fsm_connected_leave);

    if (fsm_def_state_add_transition(s, mongo_connection_fsm_connected_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_master: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

