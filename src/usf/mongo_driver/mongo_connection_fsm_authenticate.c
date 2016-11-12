#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "mongo_connection_i.h"
#include "mongo_manip_i.h"

static const char * mongo_connection_fsm_authenticate_select(mongo_connection_t connection);

static uint32_t mongo_connection_fsm_authenticate_cr_on_getnonc(mongo_connection_t connection, mongo_pkg_t pkg);
static uint32_t mongo_connection_fsm_authenticate_cr_on_authenticate(mongo_connection_t connection, mongo_pkg_t pkg);

static uint32_t mongo_connection_fsm_authenticate_scram_on_response(mongo_connection_t connection, mongo_pkg_t pkg);

static void mongo_connection_fsm_authenticate_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;
    mongo_pkg_t pkg_buf;
    const char * mechanism;
    
    if (driver->m_user[0] == 0) {
        mongo_connection_fsm_apply_recv_pkg(connection, NULL);
        return;
    }

    assert(connection->m_fd != -1);

    if (mongo_connection_start_state_timer(connection, 30 * 1000) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: authenticate: start timer fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
        return;
    }

    connection->m_sub_step = 0;
    mechanism = mongo_connection_fsm_authenticate_select(connection);
    if (strcmp(mechanism, "MONGODB-CR") == 0) {
        pkg_buf = mongo_pkg_build_cr_getnonce(driver);
        if (pkg_buf == NULL || mongo_connection_send(connection, pkg_buf) != 0) {
            CPE_ERROR(
                driver->m_em, "%s: server %s %d: authenticate: build and send getnonce fail!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_stop);
            return;
        }
    }
    else if (strcmp(mechanism, "SCRAM-SHA-1") == 0) {
        pkg_buf = mongo_pkg_build_scram_start(connection);
        if (pkg_buf == NULL || mongo_connection_send(connection, pkg_buf) != 0) {
            CPE_ERROR(
                driver->m_em, "%s: server %s %d: authenticate: build and send start fail!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_stop);
            return;
        }
    }
    else {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate: not support mechanism %s!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, mechanism);
        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_stop);
        return;
    }
    
    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: authenticate: start success, mechanism=%s!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, mechanism);
    }
}

static void mongo_connection_fsm_authenticate_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_connection_stop_state_timer(connection);
    mongo_connection_stop_watch(connection);
    mongo_connection_clear_addition(connection);
}

static uint32_t mongo_connection_fsm_authenticate_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_connection_fsm_evt * evt = input_evt;
    mongo_connection_t connection = fsm_machine_context(fsm);
    mongo_driver_t driver = connection->m_server->m_driver;

    switch(evt->m_type) {
    case mongo_connection_fsm_evt_stop:
        return mongo_connection_state_disable;

    case mongo_connection_fsm_evt_disconnected:
        return mongo_connection_state_disable;

    case mongo_connection_fsm_evt_timeout:
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: authenticate: timeout!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        return mongo_connection_state_disable;

    case mongo_connection_fsm_evt_wb_update:
        mongo_connection_start_watch(connection);
        return FSM_KEEP_STATE;
        
    case mongo_connection_fsm_evt_recv_pkg: {
        const char * mechanism;
        
        if (driver->m_user[0] == 0) return mongo_connection_state_connected;

        mechanism = mongo_connection_fsm_authenticate_select(connection);

        if (strcmp(mechanism, "MONGODB-CR") == 0) {
            switch(connection->m_sub_step) {
            case 0:
                connection->m_sub_step++;
                return mongo_connection_fsm_authenticate_cr_on_getnonc(connection, evt->m_pkg);
            case 1:
                return mongo_connection_fsm_authenticate_cr_on_authenticate(connection, evt->m_pkg);
            default:
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: authenticate: %s: unknown step %d!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, mechanism, connection->m_sub_step);
                return mongo_connection_state_disable;
            }
        }
        else if (strcmp(mechanism, "SCRAM-SHA-1") == 0) {
            return mongo_connection_fsm_authenticate_scram_on_response(connection, evt->m_pkg);
        }
        else {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: authenticate: %s: unknown mechanism!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, mechanism);
            return mongo_connection_state_disable;
        }
    }
    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_connection_fsm_create_authenticate(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "authenticate", mongo_connection_state_authenticate);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_authenticate: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_connection_fsm_authenticate_enter, mongo_connection_fsm_authenticate_leave);

    if (fsm_def_state_add_transition(s, mongo_connection_fsm_authenticate_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_authenticate: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static const char * mongo_connection_fsm_authenticate_select(mongo_connection_t connection) {
    if (connection->m_server->m_driver->m_auth_mechanism[0]) {
        return connection->m_server->m_driver->m_auth_mechanism;
    }

    if (connection->m_server->m_max_wire_version < WIRE_VERSION_SCRAM_DEFAULT) {
        return "MONGODB-CR";
    }
    
    return "SCRAM-SHA-1";
}

static uint32_t mongo_connection_fsm_authenticate_cr_on_getnonc(mongo_connection_t connection, mongo_pkg_t pkg) {
    mongo_driver_t driver = connection->m_server->m_driver;
    const char * nonce;
    char nonce_buf[64];
    bson_iter_t it;
    mongo_pkg_t pkg_buf;

    if(mongo_pkg_find(&it, pkg, 0, "nonce") != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate(cr): nonce not exist!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        return mongo_connection_state_disable;
    }
        
    nonce = bson_iter_utf8(&it, NULL);
    if (nonce == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate(cr): nonce to string fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        return mongo_connection_state_disable;
    }

    cpe_str_dup(nonce_buf, sizeof(nonce_buf), nonce);
    pkg_buf = mongo_pkg_build_cr_authenticate(driver, nonce_buf);
    if (pkg_buf == NULL || mongo_connection_send(connection, pkg_buf) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate(cr): build send authenticate pkg fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        return mongo_connection_state_disable;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: authenticate(cr): build send authenticate success!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
    }

    return FSM_KEEP_STATE;
}

static uint32_t mongo_connection_fsm_authenticate_cr_on_authenticate(mongo_connection_t connection, mongo_pkg_t pkg) {
    mongo_driver_t driver = connection->m_server->m_driver;
    bson_iter_t it;
    int32_t code;
    
    if (mongo_pkg_find(&it, pkg, 0, "code") != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate(cr): code not exist!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        return mongo_connection_state_disable;
    }
    
    code = bson_iter_int32(&it);
    if (code == 0) {
        if (driver->m_debug) { 
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: authenticate(cr): success!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        }
        return mongo_connection_state_check_readable;
    }
    else {
        if (mongo_pkg_find(&it, pkg, 0, "errmsg") == 0) {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: authenticate(cr): fail, code=%d, message=%s!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, code, bson_iter_utf8(&it, NULL));
        }
        else {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: authenticate(cr): fail, code=%d!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, code);
        }

        return mongo_connection_state_disable;
    }
}

static uint32_t mongo_connection_fsm_authenticate_scram_on_response(mongo_connection_t connection, mongo_pkg_t pkg) {
    mongo_driver_t driver = connection->m_server->m_driver;
    mem_buffer_t tmp_buffer;
    mongo_pkg_t pkg_buf;
    bson_iter_t it;
    int32_t conv_id;
    bson_subtype_t btype;
    uint32_t blen;
    const char * bstr;
    char * payload;
    
    if (mongo_pkg_find(&it, pkg, 0, "done") == 0 && bson_iter_as_bool(&it)) {
        if (driver->m_debug) { 
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: authenticate(scram): success!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
        }
        return mongo_connection_state_check_readable;
    }

    if (mongo_pkg_find(&it, pkg, 0, "conversationId") != 0 || !BSON_ITER_HOLDS_INT32(&it) || !(conv_id = bson_iter_int32(&it))
        || mongo_pkg_find(&it, pkg, 0, "payload") != 0 || !BSON_ITER_HOLDS_BINARY(&it))
    {
         const char *errmsg = "Received invalid SCRAM reply from MongoDB server.";

         if (mongo_pkg_find(&it, pkg, 0, "errmsg") == 0 && BSON_ITER_HOLDS_UTF8(&it)) {
            errmsg = bson_iter_utf8(&it, NULL);
         }
         
         CPE_ERROR(
             driver->m_em, "%s: server %s.%d: authenticate(scram): step %d: %s!",
             mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, connection->m_sub_step, errmsg);
         
         return mongo_connection_state_disable;
    }

    bson_iter_binary(&it, &btype, &blen, (const uint8_t**)&bstr);

    tmp_buffer = gd_app_tmp_buffer_at(driver->m_app, 1);
    mem_buffer_clear_data(tmp_buffer);
    mem_buffer_append(tmp_buffer, bstr, blen);
    mem_buffer_append_char(tmp_buffer, 0);
    payload = mem_buffer_make_continuous(tmp_buffer, 0);

    switch(connection->m_sub_step) {
    case 0:
        connection->m_sub_step++;
        pkg_buf = mongo_pkg_build_scram_step2(connection, conv_id, payload);
        break;
    case 1:
        connection->m_sub_step++;
        pkg_buf = mongo_pkg_build_scram_step3(connection, conv_id, payload);
        break;
    default:
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate(scram): step %d: not support step response!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, connection->m_sub_step);
        return mongo_connection_state_disable;
    }

    if (pkg_buf == NULL || mongo_connection_send(connection, pkg_buf) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s %d: authenticate(scram): step %d: build send authenticate pkg fail!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, connection->m_sub_step);
        return mongo_connection_state_disable;
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: authenticate(scram): step %d: build send authenticate pkg success!",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, connection->m_sub_step);
    }

    return FSM_KEEP_STATE;
}
