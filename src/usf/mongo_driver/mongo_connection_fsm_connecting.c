#include <assert.h>
#include <netdb.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "gd/net_dns/net_dns_task.h"
#include "gd/net_dns/net_dns_manage.h"
#include "mongo_connection_i.h"

static void mongo_connection_fsm_connecting_do_connect(mongo_connection_t connection, const char * ip);
static void mongo_connection_fsm_connecting_do_name_to_host(mongo_connection_t connection);

static void mongo_connection_fsm_connecting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;

    if (mongo_connection_start_state_timer(connection, driver->m_op_timeout_ms) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: start timer fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }

    assert(connection->m_fd == -1);
    
    if (cpe_str_is_ipv4(connection->m_server->m_host)) {
        mongo_connection_fsm_connecting_do_connect(connection, connection->m_server->m_host);
    }
    else {
        mongo_connection_fsm_connecting_do_name_to_host(connection);
    }
}

static void mongo_connection_fsm_connecting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;
    net_dns_manage_t dns_mgr;

    if ((dns_mgr = net_dns_manage_find_nc(driver->m_app, NULL))) {
        net_dns_task_free_by_ctx(dns_mgr, connection);
    }
    
    mongo_connection_stop_state_timer(connection);
    mongo_connection_stop_watch(connection);
}

static uint32_t mongo_connection_fsm_connecting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_connection_fsm_evt * evt = input_evt;
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;

    switch(evt->m_type) {
    case mongo_connection_fsm_evt_stop:
        return mongo_connection_state_disable;

    case mongo_connection_fsm_evt_disconnected:
        return mongo_connection_state_disable;

    case mongo_connection_fsm_evt_connected:
        return mongo_connection_state_check_is_master;
        
    case mongo_connection_fsm_evt_timeout:
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: connecting timeout!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        return mongo_connection_state_disable;

    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_connection_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "connecting", mongo_connection_state_connecting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_connecting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_connection_fsm_connecting_enter, mongo_connection_fsm_connecting_leave);

    if (fsm_def_state_add_transition(s, mongo_connection_fsm_connecting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_connecting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static void mongo_connection_connect_cb(EV_P_ ev_io *w, int revents) {
    mongo_connection_t connection = w->data;
    mongo_driver_t driver = connection->m_server->m_driver;
    int err;
    socklen_t err_len;

    err_len = sizeof(err);

    if (cpe_getsockopt(connection->m_fd, SOL_SOCKET, SO_ERROR, &err, &err_len) == -1) {
        CPE_ERROR(
            driver->m_em,
            "%s: server %s.%d: check state, getsockopt error, errno=%d (%s)",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
    }
    else {
        if (err == 0) {
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: connect succeed!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_connected);
        }
        else {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: connect error, errno=%d (%s)",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, err, cpe_sock_errstr(err));
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        }
    }
}

static void mongo_connection_connect_cb(EV_P_ ev_io *w, int revents);

static void mongo_connection_fsm_connecting_do_connect(mongo_connection_t connection, const char * ip) {
    mongo_driver_t driver = connection->m_server->m_driver;
    struct sockaddr_in addr;

    assert(connection->m_fd == -1);

    snprintf(connection->m_server->m_address, sizeof(connection->m_server->m_address), "%s:%d", ip, connection->m_server->m_port);
    
    connection->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, 0);
    if (connection->m_fd == -1) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: create socket fail, errno=%d (%s)!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }

    if (cpe_sock_set_none_block(connection->m_fd, 1) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: set socket none block fail, errno=%d (%s)!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(connection->m_server->m_port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (cpe_connect(connection->m_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (cpe_sock_errno() == EINPROGRESS || cpe_sock_errno() == EWOULDBLOCK) {
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: connect started!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
            assert(!ev_is_active(&connection->m_watcher));
            ev_io_init(&connection->m_watcher, mongo_connection_connect_cb, connection->m_fd, EV_WRITE);
            ev_io_start(driver->m_ev_loop, &connection->m_watcher);
            return;
        }
        else {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: connect error, errno=%d (%s)",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
                cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            cpe_sock_close(connection->m_fd);
            connection->m_fd = -1;
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
            return;
        }
    }
    else {
        CPE_INFO(
            driver->m_em, "%s: server %s.%d: connect succeed!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_connected);
        return;
    }

    return;
}

static void mongo_connection_fsm_connecting_dns_callback(void * ctx, const char * ip) {
    mongo_connection_t connection = ctx;
    mongo_driver_t driver = connection->m_server->m_driver;

    if (ip == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: resolved fail",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
    }
    else {
        CPE_INFO(
            driver->m_em, "%s: server %s.%d: resolved ip %s",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, ip);

        mongo_connection_fsm_connecting_do_connect(connection, ip);
    }
}

static void mongo_connection_fsm_connecting_do_name_to_host(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    net_dns_manage_t dns_mgr;

    dns_mgr = net_dns_manage_find_nc(driver->m_app, NULL);
    if (dns_mgr == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: start resolve host name: no dns manage",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }

    if (net_dns_ares_gethostbyname(dns_mgr, connection->m_server->m_host, mongo_connection_fsm_connecting_dns_callback, connection) == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: start resolve host name: start dns task fail",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }
    
    CPE_ERROR(
        driver->m_em, "%s: server %s.%d: start resolve host name",
        mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
}
