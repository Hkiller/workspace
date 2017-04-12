#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"

mongo_connection_t mongo_connection_create(mongo_server_t server) {
    mongo_connection_t connection;
    mongo_driver_t driver = server->m_driver;

    connection = mem_alloc(driver->m_alloc, sizeof(struct mongo_connection));
    if (connection == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: create connection: alloc fail!",
            mongo_driver_name(driver), server->m_host, server->m_port);
        return NULL;
    }

    connection->m_id = driver->m_max_connection_id + 1;
    connection->m_server = server;
    connection->m_sub_step = 0;
    connection->m_rb = NULL;
    connection->m_wb = NULL;
    connection->m_fd = -1;
    ev_init(&connection->m_watcher, NULL);
    connection->m_watcher.data = connection;
    connection->m_fsm_timer_id = GD_TIMER_ID_INVALID;
    connection->m_addition = NULL;
    connection->m_addition_cleanup = NULL;
    connection->m_to_send_pkg_count = 0;
    connection->m_sending_pkg_op = 0;
    connection->m_runing_pkg_begin = 0;
    connection->m_runing_pkg_end = 0;
    
    if (fsm_machine_init(&connection->m_fsm, driver->m_fsm_def, "disable", connection, driver->m_debug) != 0) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: create connection: init fsm fail!",
            mongo_driver_name(server->m_driver), server->m_host, server->m_port);
        mem_free(driver->m_alloc, connection);
        return NULL;
    }

    driver->m_max_connection_id++;
    connection->m_server->m_connection_count++;
    TAILQ_INSERT_TAIL(&server->m_connections, connection, m_next_for_server);

    return connection;
}

void mongo_connection_free(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;

    mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_stop);
    fsm_machine_fini(&connection->m_fsm);

    assert(connection->m_addition == NULL);
    assert(connection->m_addition_cleanup == NULL);
    
    assert(connection->m_fsm_timer_id == GD_TIMER_ID_INVALID);
    assert(connection->m_fd == -1);

    if (connection->m_rb) {
        ringbuffer_free(driver->m_ringbuf, connection->m_rb);
        connection->m_rb = NULL;
    }

    if (connection->m_wb) {
        ringbuffer_free(driver->m_ringbuf, connection->m_wb);
        connection->m_wb = NULL;
    }

    assert(connection->m_server->m_connection_count > 0);
    connection->m_server->m_connection_count--;
    TAILQ_REMOVE(&connection->m_server->m_connections, connection, m_next_for_server);

    mem_free(driver->m_alloc, connection);
}

void mongo_connection_free_all(mongo_server_t server) {
    while(!TAILQ_EMPTY(&server->m_connections)) {
        mongo_connection_free(TAILQ_FIRST(&server->m_connections));
    }
}

mongo_connection_t mongo_connection_find_by_fd(mongo_driver_t driver, int fd) {
    mongo_server_t server;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        mongo_connection_t connection;
        
        TAILQ_FOREACH(connection, &server->m_connections, m_next_for_server) {
            if (connection->m_fd == fd) return connection;
        }
    }

    return NULL;
}

void mongo_connection_fsm_apply_evt(mongo_connection_t connection, enum mongo_connection_fsm_evt_type type) {
    struct mongo_connection_fsm_evt evt;
    evt.m_type = type;
    evt.m_pkg = NULL;
    fsm_machine_apply_event(&connection->m_fsm, &evt);
}

void mongo_connection_fsm_apply_recv_pkg(mongo_connection_t connection, mongo_pkg_t pkg) {
    struct mongo_connection_fsm_evt evt;
    evt.m_type = mongo_connection_fsm_evt_recv_pkg;
    evt.m_pkg = pkg;
    fsm_machine_apply_event(&connection->m_fsm, &evt);
}

static void mongo_connection_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    mongo_connection_t connection = ctx;
    assert(connection->m_fsm_timer_id == timer_id);
    mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_timeout);
}

int mongo_connection_start_state_timer(mongo_connection_t connection, tl_time_span_t span) {
    mongo_driver_t driver = connection->m_server->m_driver;
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(driver->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: start state timer: get default timer manager fail!",
            mongo_driver_name(driver));
        return -1;
    }

    assert(connection->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &connection->m_fsm_timer_id, mongo_connection_state_timeout, connection, NULL, NULL, span, span, -1) != 0) {
        assert(connection->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(driver->m_em, "%s: start state timer: regist timer fail!", mongo_driver_name(driver));
        return -1;
    }

    assert(connection->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void mongo_connection_stop_state_timer(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    gd_timer_mgr_t timer_mgr;

    if (connection->m_fsm_timer_id == GD_TIMER_ID_INVALID) return;

    timer_mgr = gd_timer_mgr_default(driver->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(driver->m_em, "%s: start state timer: get default timer manager fail!", mongo_driver_name(driver));
        return;
    }

    assert(connection->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, connection->m_fsm_timer_id);
    connection->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}

static void mongo_connection_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct mongo_connection_fsm_evt * evt = input_event;

    switch(evt->m_type) {
    case mongo_connection_fsm_evt_start:
        stream_printf(s, "connection start");
        break;
    case mongo_connection_fsm_evt_stop:
        stream_printf(s, "connection stop");
        break;
    case mongo_connection_fsm_evt_connected:
        stream_printf(s, "connection connected");
        break;
    case mongo_connection_fsm_evt_disconnected:
        stream_printf(s, "connection disconnected");
        break;
    case mongo_connection_fsm_evt_timeout:
        stream_printf(s, "connection timeout");
        break;
    case mongo_connection_fsm_evt_recv_pkg:
        stream_printf(s, "connection recv");
        break;
    case mongo_connection_fsm_evt_wb_update:
        stream_printf(s, "write-buf update");
        break;
    default:
        stream_printf(s, "unknown server fsm evt %d", evt->m_type);
        break;
    }
}

fsm_def_machine_t mongo_connection_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    char buf[128];
    fsm_def_machine_t fsm_def;

    snprintf(buf, sizeof(buf), "%s.server", name);
    fsm_def = fsm_def_machine_create(buf, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "mongo_connection_create_fsm_def: create fsm def fail!");
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, mongo_connection_dump_event);

    if (mongo_connection_fsm_create_disable(fsm_def, em) != 0
        || mongo_connection_fsm_create_connecting(fsm_def, em) != 0
        || mongo_connection_fsm_create_check_is_master(fsm_def, em) != 0
        || mongo_connection_fsm_create_authenticate(fsm_def, em) != 0
        || mongo_connection_fsm_create_check_readable(fsm_def, em) != 0
        || mongo_connection_fsm_create_connected(fsm_def, em) != 0
        )
    {
        CPE_ERROR(em, "mongo_connection_create_fsm_def: init fsm fail!");
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

void mongo_connection_disconnect(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;

    if (connection->m_fd == -1) return;

    mongo_connection_stop_watch(connection);

    cpe_sock_close(connection->m_fd);
    connection->m_fd = -1;

    if (connection->m_rb) {
        ringbuffer_free(driver->m_ringbuf, connection->m_rb);
        connection->m_rb = NULL;
    }

    if (connection->m_wb) {
        ringbuffer_free(driver->m_ringbuf, connection->m_wb);
        connection->m_wb = NULL;
    }

    connection->m_sending_pkg_op = 0;
    connection->m_to_send_pkg_count = 0;
    connection->m_runing_pkg_begin = connection->m_runing_pkg_end = 0;
}

void mongo_connection_link_node_r(mongo_connection_t connection, ringbuffer_block_t blk) {
    if (connection->m_rb) {
		ringbuffer_link(connection->m_server->m_driver->m_ringbuf, connection->m_rb , blk);
	}
    else {
		blk->id = 1;
		connection->m_rb = blk;
	}
}

void mongo_connection_link_node_w(mongo_connection_t connection, ringbuffer_block_t blk) {
    if (connection->m_wb) {
		ringbuffer_link(connection->m_server->m_driver->m_ringbuf, connection->m_wb , blk);
	}
    else {
		blk->id = 2;
		connection->m_wb = blk;
	}
}

int mongo_connection_alloc(ringbuffer_block_t * result, mongo_driver_t driver, mongo_connection_t connection, size_t size) {
    ringbuffer_block_t blk;

    blk = ringbuffer_alloc(driver->m_ringbuf , size);
    while (blk == NULL) {
        mongo_connection_t disable_connection;
        int collect_id = ringbuffer_collect(driver->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: alloc: not enouth capacity, len=%d!",
                mongo_driver_name(driver),connection->m_server->m_host, connection->m_server->m_port, (int)size);
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
            return -1;
        }

        disable_connection = mongo_connection_find_by_fd(driver, collect_id);
        assert(disable_connection);

        CPE_INFO(
            driver->m_em, "%s: server %s.%d: alloc: not enouth free buff, disable server %s.%d!",
            mongo_driver_name(driver),connection->m_server->m_host, connection->m_server->m_port,
            disable_connection->m_server->m_host, disable_connection->m_server->m_port);
        mongo_connection_fsm_apply_evt(disable_connection, mongo_connection_fsm_evt_disconnected);
        if (disable_connection == connection) return -1;

        blk = ringbuffer_alloc(driver->m_ringbuf , size);
    }

    *result = blk;
    return 0;
}

void mongo_connection_stop_watch(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    
    if (ev_is_active(&connection->m_watcher)) ev_io_stop(driver->m_ev_loop, &connection->m_watcher);
}

void mongo_connection_start_watch(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    int events;
    
    if (ev_is_active(&connection->m_watcher)) ev_io_stop(driver->m_ev_loop, &connection->m_watcher);

    events = EV_READ;
    if (connection->m_to_send_pkg_count > 0) {
        if (mongo_connection_runing_pkg_full(connection)) {
            if (driver->m_debug) {
                CPE_INFO(
                    driver->m_em, "%s: server %s:%d: start watch, runing pkg full!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
            }
        }
        else {
            events |= EV_WRITE;
        }
    }
    
    ev_io_init(&connection->m_watcher, mongo_connection_rw_cb, connection->m_fd, events);
    ev_io_start(driver->m_ev_loop, &connection->m_watcher);
}

void mongo_connection_clear_addition(mongo_connection_t connection) {
    if (connection->m_addition && connection->m_addition_cleanup) {
        connection->m_addition_cleanup(connection->m_server->m_driver, connection->m_addition);
    }
    connection->m_addition = NULL;
    connection->m_addition_cleanup = NULL;
}

uint8_t mongo_connection_runing_pkg_count(mongo_connection_t connection) {
    if (connection->m_runing_pkg_end < connection->m_runing_pkg_begin) {
        return CPE_ARRAY_SIZE(connection->m_runing_pkg_send_times) - connection->m_runing_pkg_begin + connection->m_runing_pkg_end;
    }
    else {
        return connection->m_runing_pkg_end - connection->m_runing_pkg_begin;
    }
}

uint8_t mongo_connection_runing_pkg_full(mongo_connection_t connection) {
    return (mongo_connection_runing_pkg_count(connection) >= (CPE_ARRAY_SIZE(connection->m_runing_pkg_send_times) - 1)) ? 1 : 0;
}

void mongo_connection_runing_pkg_push(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    
    assert(!mongo_connection_runing_pkg_full(connection));

    assert(connection->m_runing_pkg_end < CPE_ARRAY_SIZE(connection->m_runing_pkg_send_times));
    connection->m_runing_pkg_send_times[connection->m_runing_pkg_end] = mongo_driver_cur_time_ms(driver);
    connection->m_runing_pkg_end++;

    if (connection->m_runing_pkg_end >= CPE_ARRAY_SIZE(connection->m_runing_pkg_send_times)) {
        connection->m_runing_pkg_end = 0;
    }

    assert(connection->m_runing_pkg_end != connection->m_runing_pkg_begin);

    if (driver->m_debug > 1) {
        CPE_INFO(
            driver->m_em, "%s: server %s:%d: pkg push, runing-pkg-count=%d",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
            mongo_connection_runing_pkg_count(connection));
    }
}

void mongo_connection_runing_pkg_pop(mongo_connection_t connection) {
    mongo_driver_t driver = connection->m_server->m_driver;
    uint64_t start_time;
    uint64_t cur_time;
    int64_t rtt = -1;
    
    assert(mongo_connection_runing_pkg_count(connection) != 0);

    assert(connection->m_runing_pkg_begin < CPE_ARRAY_SIZE(connection->m_runing_pkg_send_times));
    start_time = connection->m_runing_pkg_send_times[connection->m_runing_pkg_begin];
    cur_time = mongo_driver_cur_time_ms(connection->m_server->m_driver);
    connection->m_runing_pkg_begin++;
    if (connection->m_runing_pkg_begin >= CPE_ARRAY_SIZE(connection->m_runing_pkg_send_times)) {
        connection->m_runing_pkg_begin = 0;
    }

    if (cur_time > start_time) {
        rtt = (int64_t)(cur_time - start_time);
        mongo_server_update_rtt(connection->m_server, rtt);
    }

    if (driver->m_debug > 1) {
        CPE_INFO(
            driver->m_em, "%s: server %s:%d: pkg pop, runing-pkg-count=%d, rtt=" FMT_INT64_T,
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
            mongo_connection_runing_pkg_count(connection), rtt);
    }
}
