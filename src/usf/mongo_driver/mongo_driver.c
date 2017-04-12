#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dp/dp.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_driver_i.h"
#include "mongo_server_i.h"
#include "mongo_connection_i.h"

static void mongo_driver_clear(nm_node_t node);
static void mongo_driver_tick(EV_P_ struct ev_timer *w, int revents);

struct nm_node_type s_nm_node_type_mongo_driver = {
    "usf_mongo_driver",
    mongo_driver_clear
};

mongo_driver_t
mongo_driver_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    mongo_driver_t driver;
    nm_node_t driver_node;

    driver_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct mongo_driver));
    if (driver_node == NULL) return NULL;

    driver = (mongo_driver_t)nm_node_data(driver_node);
    bzero(driver, sizeof(struct mongo_driver));

    driver->m_app = app;
    driver->m_alloc = alloc;
    driver->m_em = em;
    driver->m_debug = 0;
    driver->m_is_enable = 0;
    driver->m_select_threshold_ms = 500;
    driver->m_ringbuf = NULL;
    driver->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    driver->m_max_connection_id = 0;
    
    driver->m_fsm_def = NULL;

    driver->m_pkg_buf_max_size = 4 * 1024;
    driver->m_pkg_buf = NULL;

    driver->m_incoming_send_to = NULL;
    driver->m_outgoing_recv_at = NULL;

    driver->m_user[0] = 0;
    driver->m_passwd[0] = 0;
    driver->m_source[0] = 0;
    driver->m_replica_set[0] = 0;
    driver->m_auth_mechanism[0] = 0;
    driver->m_topology_type = mongo_topology_type_unknown;
    driver->m_max_set_version = MONGO_NO_SET_VERSION;
    
    driver->m_server_count = 0;
    TAILQ_INIT(&driver->m_servers);

    driver->m_read_block_size = 2 * 1024;
    driver->m_reconnect_span_s = 1;
    driver->m_op_timeout_ms = 30 * 1000;

    driver->m_fsm_def = mongo_connection_create_fsm_def(name, alloc, em);
    if (driver->m_fsm_def == NULL) {
        CPE_ERROR(em, "%s: create: create server fsm fail!", name);
        nm_node_free(driver_node);
        return NULL;
    }

    ev_timer_init(&driver->m_timer_event, mongo_driver_tick, 0.001, 30.0);
    driver->m_timer_event.data = driver;
    
    mem_buffer_init(&driver->m_dump_buffer, driver->m_alloc);

    nm_node_set_type(driver_node, &s_nm_node_type_mongo_driver);

    return driver;
} 

static void mongo_driver_clear(nm_node_t node) {
    mongo_driver_t driver;

    driver = (mongo_driver_t)nm_node_data(node);

    if (ev_is_active(&driver->m_timer_event)) {
        ev_timer_stop(driver->m_ev_loop, &driver->m_timer_event);
    }
    
    if (driver->m_pkg_buf) {
        mongo_pkg_free(driver->m_pkg_buf);
        driver->m_pkg_buf = NULL;
    }

    if (driver->m_incoming_send_to) {
        mem_free(driver->m_alloc, driver->m_incoming_send_to);
        driver->m_incoming_send_to = NULL;
    }

    if (driver->m_outgoing_recv_at) {
        dp_rsp_free(driver->m_outgoing_recv_at);
        driver->m_outgoing_recv_at = NULL;
    }

    mem_buffer_clear(&driver->m_dump_buffer);

    mongo_server_free_all(driver);

    if (driver->m_ringbuf) {
        ringbuffer_delete(driver->m_ringbuf);
        driver->m_ringbuf = NULL;
    }

    if (driver->m_fsm_def) {
        fsm_def_machine_free(driver->m_fsm_def);
        driver->m_fsm_def = NULL;
    }
}

void mongo_driver_free(mongo_driver_t driver) {
    nm_node_t driver_node;
    assert(driver);

    driver_node = nm_node_from_data(driver);
    if (nm_node_type(driver_node) != &s_nm_node_type_mongo_driver) return;
    nm_node_free(driver_node);
}

mongo_driver_t
mongo_driver_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_driver) return NULL;
    return (mongo_driver_t)nm_node_data(node);
}

mongo_driver_t
mongo_driver_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "mongo_driver";
    
    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_driver) return NULL;
    return (mongo_driver_t)nm_node_data(node);
}

gd_app_context_t mongo_driver_app(mongo_driver_t driver) {
    return driver->m_app;
}

const char * mongo_driver_name(mongo_driver_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

static int mongo_driver_set_uri_server(mongo_driver_t driver, const char * server_begin, const char * server_end) {
    const char * sep = strchr(server_begin, ':');
    char host[64];
    char port[32];
    
    if (sep && sep < server_end) {
        cpe_str_dup_range(host, sizeof(host), server_begin, sep);
        cpe_str_dup_range(port, sizeof(port), sep + 1, server_end);
    }
    else {
        cpe_str_dup_range(host, sizeof(host), server_begin, server_end);
        port[0] = 0;
    }

    if (mongo_server_create(driver, host, atoi(port), mongo_server_source_uri) == NULL) {
        CPE_ERROR(driver->m_em, "%s: set uri: create server %s:%s fail!", mongo_driver_name(driver), host, port);
        return -1;
    }

    CPE_INFO(driver->m_em, "%s: set uri: create server %s:%s success!", mongo_driver_name(driver), host, port);
    
    return 0;
}

int mongo_driver_set_uri(mongo_driver_t driver, const char * uri) {
    const char * sep;
    const char * sep2;

    if (driver->m_is_enable) {
        CPE_ERROR(driver->m_em, "%s: set uri: is already enabled!", mongo_driver_name(driver));
        return -1;
    }

    mongo_server_free_all(driver);
    driver->m_user[0] = 0;
    driver->m_passwd[0] = 0;
    driver->m_source[0] = 0;
    driver->m_replica_set[0] = 0;
    driver->m_auth_mechanism[0] = 0;
    driver->m_topology_type = mongo_topology_type_unknown;
    driver->m_max_set_version = MONGO_NO_SET_VERSION;
    bson_oid_copy_unsafe(&mongo_driver_oid_zero, &driver->m_max_election_id);
    
    if (!cpe_str_start_with(uri, "mongodb://")) {
        CPE_ERROR(driver->m_em, "%s: set uri: ui %s format error!", mongo_driver_name(driver), uri);
        return -1;
    }
    uri += 10;

    if ((sep = strchr(uri, '@'))) {
        if ((sep2 = strchr(uri, ':'))) {
            cpe_str_dup_range(driver->m_user, sizeof(driver->m_user), uri, sep2);
            cpe_str_dup_range(driver->m_passwd, sizeof(driver->m_passwd), sep2 + 1, sep);
        }
        else {
            cpe_str_dup_range(driver->m_user, sizeof(driver->m_user), uri, sep);
        }
        uri = sep + 1;
    }

    if ((sep = strchr(uri, '/'))) {
        while((sep2 = strchr(uri, ','))) {
            if (mongo_driver_set_uri_server(driver, uri, sep2) != 0) return -1;
            uri = sep2 + 1;
        }
        if (mongo_driver_set_uri_server(driver, uri, sep) != 0) return -1;
        uri = sep + 1;
    }
    else {
        sep = uri + strlen(uri);
        while((sep2 = strchr(uri, ','))) {
            if (mongo_driver_set_uri_server(driver, uri, sep2) != 0) return -1;
            uri = sep2 + 1;
        }
        if (mongo_driver_set_uri_server(driver, uri, sep) != 0) return -1;
        uri = sep;
    }

    if ((sep = strchr(uri, '?'))) {
        cpe_str_dup_range(driver->m_source, sizeof(driver->m_source), uri, sep);
        uri = sep + 1;
    }
    else {
        cpe_str_dup(driver->m_source, sizeof(driver->m_source), uri);
    }

    cpe_str_read_arg(
        driver->m_replica_set, sizeof(driver->m_replica_set),
        uri, "replicaSet", ',', '=');

    /*更新topo结构设置 */
    if (driver->m_replica_set[0]) {
        driver->m_topology_type = mongo_topology_type_rs_no_primary;
    }
    else {
        if (driver->m_server_count > 1) {
            driver->m_topology_type = mongo_topology_type_unknown;
        }
        else {
            driver->m_topology_type = mongo_topology_type_single;
        }
   }
    
    CPE_INFO(
        driver->m_em, "%s: set uri: user=%s, passwd=%s, source=%s, replica-set=%s, server-count=%d!",
        mongo_driver_name(driver), driver->m_user, driver->m_passwd, driver->m_source, driver->m_replica_set, driver->m_server_count);
    
    return 0;
}

int mongo_driver_set_incoming_send_to(mongo_driver_t driver, const char * incoming_send_to) {
    if (incoming_send_to == NULL) {
        if (driver->m_incoming_send_to) {
            mem_free(driver->m_alloc, driver->m_incoming_send_to);
            driver->m_incoming_send_to = NULL;
        }
    }
    else {
        size_t name_len;
        cpe_hash_string_t buf;
        
        name_len = cpe_hs_len_to_binary_len(strlen(incoming_send_to));
        buf = mem_alloc(driver->m_alloc, name_len);
        if (buf == NULL) return -1;

        cpe_hs_init(buf, name_len, incoming_send_to);

        if (driver->m_incoming_send_to) mem_free(driver->m_alloc, driver->m_incoming_send_to);

        driver->m_incoming_send_to = buf;
    }
    
    return 0;
}

int mongo_driver_set_outgoing_recv_at(mongo_driver_t driver, const char * outgoing_recv_at) {
    if (outgoing_recv_at == NULL) {
        if (driver->m_outgoing_recv_at) {
            dp_rsp_free(driver->m_outgoing_recv_at);
            driver->m_outgoing_recv_at = NULL;
        }
    }
    else {
        char name_buf[128];

        snprintf(name_buf, sizeof(name_buf), "%s.outgoing-recv-rsp", mongo_driver_name(driver));

        if (driver->m_outgoing_recv_at) dp_rsp_free(driver->m_outgoing_recv_at);

        driver->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(driver->m_app), name_buf);
        if (driver->m_outgoing_recv_at == NULL) return -1;

        dp_rsp_set_processor(driver->m_outgoing_recv_at, mongo_driver_on_send, driver);

        if (dp_rsp_bind_string(driver->m_outgoing_recv_at, outgoing_recv_at, driver->m_em) != 0) {
            CPE_ERROR(
                driver->m_em, "%s: mongo_driver_set_outgoing_recv_at: bing to %s fail!",
                mongo_driver_name(driver), outgoing_recv_at);
            dp_rsp_free(driver->m_outgoing_recv_at);
            driver->m_outgoing_recv_at = NULL;
            return -1;
        }
    }
    
    return 0;
}

cpe_hash_string_t
mongo_driver_name_hs(mongo_driver_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

mongo_pkg_t mongo_driver_pkg_buf(mongo_driver_t driver) {
    if (driver->m_pkg_buf) {
        if (mongo_pkg_capacity(driver->m_pkg_buf) < driver->m_pkg_buf_max_size) {
            mongo_pkg_free(driver->m_pkg_buf);
            driver->m_pkg_buf = NULL;
        }
    }

    if (driver->m_pkg_buf == NULL) {
        driver->m_pkg_buf = mongo_pkg_create(driver, driver->m_pkg_buf_max_size);
    }

    mongo_pkg_init(driver->m_pkg_buf);

    return driver->m_pkg_buf;
}

int mongo_driver_enable(mongo_driver_t driver) {
    mongo_server_t server;

    if (driver->m_is_enable) return 0;

    if (TAILQ_EMPTY(&driver->m_servers)) {
        CPE_ERROR(
            driver->m_em, "%s: enable: no any server, can`t enable!",
            mongo_driver_name(driver));
        return 0;
    }

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        mongo_connection_t connection = mongo_connection_create(server);
        if (connection == NULL) {
            CPE_ERROR(
                driver->m_em, "%s: enable: server %s.%d: create connection fail!!",
                mongo_driver_name(driver), server->m_host, server->m_port);
            goto ENABLE_FAIL;
        }

        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_start);

        CPE_INFO(
            driver->m_em, "%s: enable: server %s.%d: create connection success",
            mongo_driver_name(driver), server->m_host, server->m_port);
    }

    driver->m_is_enable = 1;
    return 0;

ENABLE_FAIL:            
    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        mongo_connection_free_all(server);
    }

    return -1;
}

uint8_t mongo_driver_is_enable(mongo_driver_t driver) {
    return driver->m_is_enable;
}

uint32_t mongo_driver_cur_time_s(mongo_driver_t driver) {
    return (uint32_t) tl_manage_time(gd_app_tl_mgr(driver->m_app)) / 1000;
}

uint64_t mongo_driver_cur_time_ms(mongo_driver_t driver) {
    return tl_manage_time(gd_app_tl_mgr(driver->m_app));
}

int mongo_driver_add_server(mongo_driver_t driver, const char * host, int port) {
    return mongo_server_create(driver, host, port, mongo_server_source_uri) == NULL ? -1 : 0;
}

int mongo_driver_set_ringbuf_size(mongo_driver_t driver, size_t capacity) {
    //TODO: disconnect all :)

    if (driver->m_ringbuf) {
        ringbuffer_delete(driver->m_ringbuf);
    }
    driver->m_ringbuf = ringbuffer_new(capacity);

    if (driver->m_ringbuf == NULL) return -1;

    return 0;
}

mongo_topology_type_t mongo_driver_topology_type(mongo_driver_t driver) {
    return driver->m_topology_type;
}

int32_t mongoc_driver_lowest_max_wire_version(mongo_driver_t driver) {
   int32_t ret = INT32_MAX;
   mongo_server_t server;

   TAILQ_FOREACH(server, &driver->m_servers, m_next) {
       if (server->m_mode != mongo_server_runing_mode_unknown && server->m_max_wire_version < ret) {
           ret = server->m_max_wire_version;
       }
   }

   return ret;
}

static void mongo_driver_tick(EV_P_ struct ev_timer *w, int revents) {
    mongo_driver_t driver = w->data;
    mongo_server_t server, next_server;
    uint32_t online_server_count = 0;
    
    for(server = TAILQ_FIRST(&driver->m_servers); server; server = next_server) {
        next_server = TAILQ_NEXT(server, m_next);
        
        if (server->m_is_offline) {
            if (server->m_source == mongo_server_source_uri) {
                if (!TAILQ_EMPTY(&server->m_connections)) {
                    CPE_INFO(
                        driver->m_em, "%s: server %s:%d: is already offline, clear all connections",
                        mongo_driver_name(driver), server->m_host, server->m_port);
                    mongo_connection_free_all(server);
                }
            }
            else {
                CPE_INFO(
                    driver->m_em, "%s: server %s:%d: is already offline, auto free ",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                mongo_server_free(server);
            }
        }
        else {
            online_server_count++;
            
            if (TAILQ_EMPTY(&server->m_connections)) {
                CPE_INFO(
                    driver->m_em, "%s: server %s:%d: auto start ",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                mongo_connection_create(server);
            }
            else {
                mongo_connection_t connection;

                TAILQ_FOREACH(connection, &server->m_connections, m_next_for_server) {
                    if (fsm_machine_curent_state(&connection->m_fsm) == mongo_connection_state_disable) {
                        CPE_INFO(
                            driver->m_em, "%s: server %s:%d: connection %d auto reconnect ",
                            mongo_driver_name(driver), server->m_host, server->m_port, connection->m_id);
                        mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_start);
                    }
                }
            }
        }
    }

    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: tick: topo=%s, online-server=%d, offline-server=%d",
            mongo_driver_name(driver), mongo_topology_type_to_str(driver->m_topology_type),
            online_server_count, (driver->m_server_count - online_server_count));
    }
}

void mongo_driver_tick_once(mongo_driver_t driver) {
    if (ev_is_active(&driver->m_timer_event)) {
        ev_timer_stop(driver->m_ev_loop, &driver->m_timer_event);
    }
    ev_timer_start(driver->m_ev_loop, &driver->m_timer_event);
}

bson_oid_t mongo_driver_oid_zero = { {0} };
