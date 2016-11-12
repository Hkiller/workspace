#include <assert.h> 
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "conn_svr_ops.h"

void conn_svr_rw_cb(EV_P_ ev_io *w, int revents);

conn_svr_conn_t
conn_svr_conn_create(conn_svr_t svr, int fd, uint32_t conn_id) {
    conn_svr_conn_t conn;
    conn = mem_calloc(svr->m_alloc, sizeof(struct conn_svr_conn));
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: create conn: malloc fail!", conn_svr_name(svr));
        return NULL;
    }

    conn->m_svr = svr;
    conn->m_fd = fd;
    conn->m_data.conn_id = conn_id;
    conn->m_data.user_id = 0;
    conn->m_auth = 0;
    conn->m_rb = NULL;
    conn->m_wb = NULL;
    conn->m_last_op_time = conn_svr_cur_time(svr);

    cpe_hash_entry_init(&conn->m_hh_for_conn_id);
    if (cpe_hash_table_insert_unique(&svr->m_conns_by_conn_id, conn) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): insert fail, conn_id is already exist!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
        mem_free(svr->m_alloc, conn);
        return NULL;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): created!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
    }

    conn->m_watcher.data = conn;
    conn_svr_conn_start_watch(conn);

    TAILQ_INSERT_TAIL(&svr->m_conns_check, conn, m_next_for_check);

    return conn;
}

void conn_svr_conn_free(conn_svr_conn_t conn) {
    conn_svr_t svr = conn->m_svr;
    assert(svr);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
    }

    ev_io_stop(svr->m_ev_loop, &conn->m_watcher);
    cpe_sock_close(conn->m_fd);

    if (conn->m_rb) {
        ringbuffer_free(svr->m_ringbuf, conn->m_rb);
        conn->m_rb = NULL;
    }

    if (conn->m_wb) {
        ringbuffer_free(svr->m_ringbuf, conn->m_wb);
        conn->m_wb = NULL;
    }

    TAILQ_REMOVE(&svr->m_conns_check, conn, m_next_for_check);

    if (conn->m_data.user_id) {
        cpe_hash_table_remove_by_ins(&svr->m_conns_by_user_id, conn);
    }

    cpe_hash_table_remove_by_ins(&svr->m_conns_by_conn_id, conn);

    mem_free(svr->m_alloc, conn);
}

void conn_svr_conn_free_all(conn_svr_t svr) {
    struct cpe_hash_it conn_it;
    conn_svr_conn_t conn;

    cpe_hash_it_init(&conn_it, &svr->m_conns_by_conn_id);
    conn = cpe_hash_it_next(&conn_it);
    while(conn) {
        conn_svr_conn_t next = cpe_hash_it_next(&conn_it);
        conn_svr_conn_free(conn);
        conn = next;
    }
}

void conn_svr_conn_update_op_time(conn_svr_conn_t conn) {
    conn_svr_t svr = conn->m_svr;

    TAILQ_REMOVE(&svr->m_conns_check, conn, m_next_for_check);
    conn->m_last_op_time = conn_svr_cur_time(svr);
    TAILQ_INSERT_TAIL(&svr->m_conns_check, conn, m_next_for_check);
}

int conn_svr_conn_set_user_info(conn_svr_conn_t conn, CONN_SVR_CONN_INFO const * data) {
    conn_svr_t svr = conn->m_svr;

    assert(conn->m_data.conn_id == data->conn_id);

    if (conn->m_data.user_id) {
        cpe_hash_table_remove_by_ins(&svr->m_conns_by_user_id, conn);

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): unbind to user "FMT_UINT64_T" success!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, conn->m_data.user_id);
        }
    }

    conn->m_data = *data;
        
    if (conn->m_data.user_id) {
        cpe_hash_entry_init(&conn->m_hh_for_user_id);
        if (cpe_hash_table_insert_unique(&svr->m_conns_by_user_id, conn) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): bind to user "FMT_UINT64_T" fail, binding already exist!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, data->user_id);
            conn->m_data.user_id = 0;
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): bind to user "FMT_UINT64_T" success!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, data->user_id);
        }
    }

    return 0;
}

conn_svr_conn_t
conn_svr_conn_find_by_conn_id(conn_svr_t svr, uint32_t conn_id) {
    struct conn_svr_conn key;

    key.m_data.conn_id = conn_id;

    return cpe_hash_table_find(&svr->m_conns_by_conn_id, &key);
}

conn_svr_conn_t
conn_svr_conn_find_by_user_id(conn_svr_t svr, uint64_t user_id) {
    struct conn_svr_conn key;

    key.m_data.user_id = user_id;

    return cpe_hash_table_find(&svr->m_conns_by_user_id, &key);
}

void conn_svr_conn_start_watch(conn_svr_conn_t conn) {
    ev_io_init(&conn->m_watcher, conn_svr_rw_cb, conn->m_fd, conn->m_wb ? (EV_READ | EV_WRITE) : EV_READ);
    ev_io_start(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

void conn_svr_conn_link_node_r(conn_svr_conn_t conn, ringbuffer_block_t blk) {
    if (conn->m_rb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, conn->m_rb , blk);
	}
    else {
		blk->id = conn->m_data.conn_id;
		conn->m_rb = blk;
	}
}

void conn_svr_conn_link_node_w(conn_svr_conn_t conn, ringbuffer_block_t blk) {
    if (conn->m_wb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, conn->m_wb , blk);
	}
    else {
		blk->id = conn->m_data.conn_id;
		conn->m_wb = blk;
	}
}

int conn_svr_conn_alloc(ringbuffer_block_t * result, conn_svr_t svr, conn_svr_conn_t conn, size_t size) {
    ringbuffer_block_t blk;

    blk = ringbuffer_alloc(svr->m_ringbuf , size);
    while (blk == NULL) {
        conn_svr_conn_t free_conn;
        int collect_id = ringbuffer_collect(svr->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d): alloc: not enouth capacity, len=%d!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, (int)size);
            conn_svr_conn_free(conn);
            return -1;
        }

        free_conn = conn_svr_conn_find_by_conn_id(svr, collect_id);
        assert(free_conn);

        CPE_INFO(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d): alloc: not enouth free buff, free conn(conn_id=%d, fd=%d)!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, free_conn->m_data.conn_id, free_conn->m_fd);
        conn_svr_conn_free(free_conn);
        if (free_conn == conn) return -1;

        blk = ringbuffer_alloc(svr->m_ringbuf , size);
    }

    *result = blk;
    return 0;
}

uint32_t conn_svr_conn_user_id_hash(conn_svr_conn_t conn) {
    return (uint32_t)conn->m_data.user_id;
}

int conn_svr_conn_user_id_eq(conn_svr_conn_t l, conn_svr_conn_t r) {
    return l->m_data.user_id == r->m_data.user_id;
}

uint32_t conn_svr_conn_conn_id_hash(conn_svr_conn_t conn) {
    return (uint32_t)conn->m_data.conn_id;
}

int conn_svr_conn_conn_id_eq(conn_svr_conn_t l, conn_svr_conn_t r) {
    return l->m_data.conn_id == r->m_data.conn_id;
}
