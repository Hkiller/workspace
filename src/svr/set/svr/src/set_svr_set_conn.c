#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "set_svr_set_conn.h"
#include "set_svr_set_conn_fsm.h"

set_svr_set_conn_t
set_svr_set_conn_create(set_svr_t svr, set_svr_set_t set, int fd) {
    set_svr_set_conn_t conn;
    const char * init_state;

    conn = mem_alloc(svr->m_alloc, sizeof(struct set_svr_set_conn));
    if (conn == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: create conn: init fsm fail!",
            set_svr_name(svr), set_svr_set_name(set));
        return NULL;
    }

    conn->m_conn_id = svr->m_max_conn_id + 1;
    conn->m_svr = svr;
    conn->m_set = set;
    conn->m_fd = fd;
    conn->m_watcher.data = conn;
    conn->m_fsm_timer_id = GD_TIMER_ID_INVALID;
    conn->m_wb = NULL;
    conn->m_rb = NULL;
    conn->m_tb = NULL;

    if (conn->m_fd == -1) {
        init_state = "connecting";
    }
    else {
        init_state = "accepting";
    }

    if (fsm_machine_init(&conn->m_fsm, svr->m_set_conn_fsm_def, init_state, conn, svr->m_debug >= 2 ? 1 : 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: conn create: init fsm fail!",
            set_svr_name(svr), set_svr_set_name(conn->m_set));
        mem_free(svr->m_alloc, conn);
        return NULL;
    }

    if (set) {
        assert(set->m_conn == NULL);
        set->m_conn = conn;
    }
    else {
        TAILQ_INSERT_TAIL(&svr->m_accept_set_conns, conn, m_next);
    }

    if (svr->m_debug >= 2) {
        if (set) {
            CPE_INFO(
                svr->m_em, "%s: set %s: conn create!",
                set_svr_name(svr), set_svr_set_name(set));
        }
        else {
            CPE_INFO(svr->m_em, "%s: fd %d: conn create!!", set_svr_name(svr), conn->m_fd);
        }
    }

    svr->m_max_conn_id++;
    
    return conn;
}

void set_svr_set_conn_free(set_svr_set_conn_t conn) {
    set_svr_t svr = conn->m_svr;

    fsm_machine_fini(&conn->m_fsm);
    assert(conn->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (conn->m_rb) {
        ringbuffer_free(svr->m_ringbuf, conn->m_rb);
        conn->m_rb = NULL;
    }

    if (conn->m_wb) {
        ringbuffer_free(svr->m_ringbuf, conn->m_wb);
        conn->m_wb = NULL;
    }

    if (conn->m_tb) {
        ringbuffer_free(svr->m_ringbuf, conn->m_tb);
        conn->m_tb = NULL;
    }

    if (conn->m_set) {
        assert(conn->m_set->m_conn);
        assert(conn->m_set->m_conn == conn);
        conn->m_set->m_conn = NULL;
    }
    else {
        TAILQ_REMOVE(&svr->m_accept_set_conns, conn, m_next);
    }

    if (svr->m_debug >= 2) {
        CPE_INFO(
            svr->m_em, "%s: set %s: fd %d: conn free!",
            set_svr_name(svr), set_svr_set_name(conn->m_set), conn->m_fd);
    }

    if (conn->m_fd != -1) {
        cpe_sock_close(conn->m_fd);
        conn->m_fd = -1;
    }

    mem_free(svr->m_alloc, conn);
}

void set_svr_set_conn_set_set(set_svr_set_conn_t conn, set_svr_set_t set) {
    set_svr_t svr = conn->m_svr;

    assert(set->m_conn == NULL);
    assert(conn->m_set == NULL);

    TAILQ_REMOVE(&svr->m_accept_set_conns, conn, m_next);
    conn->m_set = set;
    set->m_conn = conn;
}

void set_svr_set_conn_apply_evt(set_svr_set_conn_t conn, enum set_svr_set_conn_fsm_evt_type type) {
    struct set_svr_set_conn_fsm_evt evt;
    evt.m_type = type;
    evt.m_pkg = NULL;
    fsm_machine_apply_event(&conn->m_fsm, &evt);
}

void set_svr_set_conn_apply_pkg(set_svr_set_conn_t conn, SVR_CENTER_PKG * pkg) {
    struct set_svr_set_conn_fsm_evt evt;
    evt.m_type = set_svr_set_conn_fsm_evt_pkg;
    evt.m_pkg = pkg;
    fsm_machine_apply_event(&conn->m_fsm, &evt);
}

static void set_svr_set_conn_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    set_svr_set_conn_t conn = ctx;
    assert(conn->m_fsm_timer_id == timer_id);
    set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_timeout);
}

int set_svr_set_conn_start_state_timer(set_svr_set_conn_t conn, tl_time_span_t span) {
    set_svr_t svr = conn->m_svr;
    gd_timer_mgr_t timer_mgr;

    timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: start state timer: get default timer manager fail!",
            set_svr_name(svr), set_svr_set_name(conn->m_set));
        return -1;
    }

    assert(conn->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(
            timer_mgr, &conn->m_fsm_timer_id, set_svr_set_conn_state_timeout,
            conn, NULL, NULL, span, span, -1) != 0)
    {
        assert(conn->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(
            svr->m_em, "%s: set %s: start state timer: regist timer fail!",
            set_svr_name(svr), set_svr_set_name(conn->m_set));
        return -1;
    }

    assert(conn->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void set_svr_set_conn_stop_state_timer(set_svr_set_conn_t conn) {
    set_svr_t svr = conn->m_svr;
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: stop state timer: get default timer manager fail!",
            set_svr_name(svr), set_svr_set_name(conn->m_set));
        return;
    }

    assert(conn->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, conn->m_fsm_timer_id);
    conn->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}

static void set_svr_set_conn_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct set_svr_set_conn_fsm_evt * evt = input_event;
    switch(evt->m_type) {
    case set_svr_set_conn_fsm_evt_pkg:
        stream_printf(s, "package: ");
        break;
    case set_svr_set_conn_fsm_evt_timeout:
        stream_printf(s, "timeout");
        break;
    case set_svr_set_conn_fsm_evt_connected:
        stream_printf(s, "connected");
        break;
    case set_svr_set_conn_fsm_evt_disconnected:
        stream_printf(s, "disconnected");
    case set_svr_set_conn_fsm_evt_accepted:
        stream_printf(s, "accepted");
        break;
    case set_svr_set_conn_fsm_evt_registed:
        stream_printf(s, "registed");
        break;
    case set_svr_set_conn_fsm_evt_wb_update:
        stream_printf(s, "wb_update");
        break;
    }
}

fsm_def_machine_t
set_svr_set_conn_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    fsm_def_machine_t fsm_def = fsm_def_machine_create(name, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "%s: create fsm def fail!", name);
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, set_svr_set_conn_dump_event);

    if (set_svr_set_conn_fsm_create_connecting(fsm_def, em) != 0
        || set_svr_set_conn_fsm_create_established(fsm_def, em) != 0
        || set_svr_set_conn_fsm_create_registing(fsm_def, em) != 0
        || set_svr_set_conn_fsm_create_accepting(fsm_def, em) != 0)
    {
        CPE_ERROR(em, "%s: init fsm fail!", name);
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

void set_svr_set_conn_link_node_r(set_svr_set_conn_t conn, ringbuffer_block_t blk) {
    if (conn->m_rb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, conn->m_rb , blk);
	}
    else {
		blk->id = conn->m_conn_id;
		conn->m_rb = blk;
	}
}

void set_svr_set_conn_link_node_w_head(set_svr_set_conn_t conn, ringbuffer_block_t blk) {
    blk->id = conn->m_conn_id;
    
    if (conn->m_wb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, blk, conn->m_wb);
	}

    conn->m_wb = blk;
}

void set_svr_set_conn_link_node_w_tail(set_svr_set_conn_t conn, ringbuffer_block_t blk) {
    if (conn->m_wb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, conn->m_wb , blk);
	}
    else {
		blk->id = conn->m_conn_id;
		conn->m_wb = blk;
	}
}

int set_svr_set_conn_read_from_net(set_svr_set_conn_t conn, size_t require_size) {
    set_svr_t svr = conn->m_svr;
    set_svr_set_t set = conn->m_set;
    ringbuffer_block_t blk;
    char * buffer;

    blk = set_svr_ringbuffer_alloc(svr, require_size, conn->m_conn_id);
    if (blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: fd %d: recv not enouth ringbuf, len=%d!",
            set_svr_name(svr), set_svr_set_name(conn->m_set), conn->m_fd,
            (int)svr->m_set_read_block_size);
        return -1;
    }

    buffer = NULL;
    ringbuffer_block_data(svr->m_ringbuf, blk, 0, (void **)&buffer);
    assert(buffer);

    for(;;) {
        int bytes = cpe_recv(conn->m_fd, buffer, require_size, 0);
        if (bytes > 0) {
            if (svr->m_debug >= 2) {
                CPE_INFO(
                    svr->m_em, "%s: set %s: fd %d: recv %d bytes data!",
                    set_svr_name(svr), set_svr_set_name(set), conn->m_fd, bytes);
            }

            ringbuffer_shrink(svr->m_ringbuf, blk, bytes);
            set_svr_set_conn_link_node_r(conn, blk);
            break;
        }
        else if (bytes == 0) {
            blk = ringbuffer_yield(svr->m_ringbuf, blk, require_size);
            assert(blk == NULL);
            CPE_ERROR(
                svr->m_em, "%s: set %s: fd %d: free for recv return 0!",
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd);
            return -1;
        }
        else {
            assert(bytes == -1);

            switch(errno) {
            case EWOULDBLOCK:
            case EINPROGRESS:
                blk = ringbuffer_yield(svr->m_ringbuf, blk, svr->m_set_read_block_size);
                assert(blk == NULL);
                break;
            case EINTR:
                continue;
            default:
                blk = ringbuffer_yield(svr->m_ringbuf, blk, svr->m_set_read_block_size);
                assert(blk == NULL);
                CPE_ERROR(
                    svr->m_em, "%s: set %s: fd %d: free for recv error, errno=%d (%s)!",
                    set_svr_name(svr), set_svr_set_name(set), conn->m_fd,
                    cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                return -1;
            }
        }
    }

    return 0;
}

int set_svr_set_conn_write_to_net(set_svr_set_conn_t conn) {
    set_svr_t svr = conn->m_svr;
    set_svr_set_t set = conn->m_set;

    while(conn->m_wb) {
        void * data;
        int block_size;
        int bytes;

        block_size = ringbuffer_block_data(svr->m_ringbuf, conn->m_wb, 0, &data);
        assert(block_size > 0);
        assert(data);

        bytes = cpe_send(conn->m_fd, data, block_size, 0);
        if (bytes > 0) {
            if (svr->m_debug >= 2) {
                CPE_INFO(
                    svr->m_em, "%s: set %s: fd %d: send %d bytes data!",
                    set_svr_name(svr), set_svr_set_name(set), conn->m_fd, bytes);
            }

            conn->m_wb = ringbuffer_yield(svr->m_ringbuf, conn->m_wb, bytes);
            if (bytes < block_size) break;
        }
        else if (bytes == 0) {
            CPE_ERROR(
                svr->m_em, "%s: set %s: fd %d: free for send return 0!",
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd);
            return -1;
        }
        else {
            int err = cpe_sock_errno();
            assert(bytes == -1);

            if (err == EWOULDBLOCK || err == EINPROGRESS) break;
            if (err == EINTR) continue;

            CPE_ERROR(
                svr->m_em, "%s: set %s: fd %d: free for send error, errno=%d (%s)!",
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd,
                err, cpe_sock_errstr(err));

            return -1;
        }
    }

    return 0;
}

static void * set_svr_set_conn_merge_rb(set_svr_set_conn_t conn) {
    set_svr_t svr = conn->m_svr;
    int length = ringbuffer_block_total_len(svr->m_ringbuf, conn->m_rb);
    set_svr_set_t set = conn->m_set;
    ringbuffer_block_t new_blk;
    void * buf;

    assert(set);

    new_blk = set_svr_ringbuffer_alloc(svr, length, conn->m_conn_id);
    if (new_blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: fd %d: recv: not enouth ringbuf, len=%d!",
            set_svr_name(svr), set_svr_set_name(set), conn->m_fd, (int)length);
        return NULL;
    }

    buf = ringbuffer_copy(svr->m_ringbuf, conn->m_rb, 0, new_blk);
    assert(buf);

    ringbuffer_free(svr->m_ringbuf, conn->m_rb);
    new_blk->id = conn->m_conn_id;
    conn->m_rb = new_blk;

    return buf;
}

int set_svr_set_conn_r_buf(set_svr_set_conn_t conn, size_t require_size, void * * buf) {
    set_svr_t svr = conn->m_svr;
    int received_size;

    if (conn->m_rb == NULL) {
        *buf = NULL;
        return 0;
    }

    received_size = ringbuffer_data(svr->m_ringbuf, conn->m_rb, require_size, 0, buf);
    if (received_size < require_size) return received_size;

    if (*buf == NULL) {
        *buf = set_svr_set_conn_merge_rb(conn);
        if (*buf == NULL) return -1;
    }

    return received_size;
}

void set_svr_set_conn_r_erase(set_svr_set_conn_t conn, size_t size) {
    assert(conn->m_rb);
    conn->m_rb = ringbuffer_yield(conn->m_svr->m_ringbuf, conn->m_rb, size);
}

