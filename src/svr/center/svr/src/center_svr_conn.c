#include <assert.h> 
#include "cpe/pal/pal_socket.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_endpoint.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "center_svr_conn.h"
#include "center_svr_set_proxy.h"

center_svr_conn_t
center_svr_conn_create(center_svr_t svr, int fd) {
    center_svr_conn_t conn;

    assert(fd != -1);

    conn = mem_alloc(svr->m_alloc, sizeof(struct center_svr_conn));
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: conn %d: malloc fail!", center_svr_name(svr), fd);
        return NULL;
    }

    conn->m_svr = svr;
    conn->m_set = NULL;
    conn->m_fd = fd;
    conn->m_rb = NULL;
    conn->m_wb = NULL;
    conn->m_tb = NULL;

    conn->m_watcher.data = conn;
    center_svr_conn_start_watch(conn);

    cpe_hash_entry_init(&conn->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_conns, conn) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: creeate: insert fail, already exist!",
            center_svr_name(svr), fd);
        mem_free(svr->m_alloc, conn);
        return NULL;
    }

    if(svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: conn %d: create", center_svr_name(svr), conn->m_fd);
    }

    return conn;
}

void center_svr_conn_free(center_svr_conn_t conn) {
    center_svr_t svr = conn->m_svr;
    assert(svr);
    assert(conn->m_fd != -1);

    if(svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: conn %d: free", center_svr_name(svr), conn->m_fd);
    }

    cpe_hash_table_remove_by_ins(&svr->m_conns, conn);

    ev_io_stop(svr->m_ev_loop, &conn->m_watcher);
    cpe_sock_close(conn->m_fd);
    conn->m_fd = -1;

    if (conn->m_set) {
        assert(conn->m_set->m_conn == conn);
        conn->m_set->m_conn = NULL;
    }

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
        conn->m_wb = NULL;
    }

    mem_free(svr->m_alloc, conn);
}

void center_svr_conn_free_all(center_svr_t svr) {
    struct cpe_hash_it conn_it;
    center_svr_conn_t conn;

    cpe_hash_it_init(&conn_it, &svr->m_conns);

    conn = cpe_hash_it_next(&conn_it);
    while(conn) {
        center_svr_conn_t next = cpe_hash_it_next(&conn_it);
        center_svr_conn_free(conn);
        conn = next;
    }
}

center_svr_conn_t center_svr_conn_find_by_fd(center_svr_t svr, int fd) {
    struct center_svr_conn key;
    key.m_fd = fd;
    return cpe_hash_table_find(&svr->m_conns, &key);
}

extern void center_svr_conn_rw_cb(EV_P_ ev_io *w, int revents);
void center_svr_conn_start_watch(center_svr_conn_t conn) {
    ev_io_init(&conn->m_watcher, center_svr_conn_rw_cb, conn->m_fd, conn->m_wb ? (EV_READ | EV_WRITE) : EV_READ);
    ev_io_start(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

void center_svr_conn_link_node_r(center_svr_conn_t conn, ringbuffer_block_t blk) {
    if (conn->m_rb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, conn->m_rb , blk);
	}
    else {
		blk->id = conn->m_fd;
		conn->m_rb = blk;
	}
}

void center_svr_conn_link_node_w(center_svr_conn_t conn, ringbuffer_block_t blk) {
    if (conn->m_wb) {
		ringbuffer_link(conn->m_svr->m_ringbuf, conn->m_wb , blk);
	}
    else {
		blk->id = conn->m_fd;
		conn->m_wb = blk;
	}
}

int center_svr_conn_alloc(ringbuffer_block_t * result, center_svr_t svr, center_svr_conn_t conn, size_t size) {
    ringbuffer_block_t blk;

    blk = ringbuffer_alloc(svr->m_ringbuf , size);
    while (blk == NULL) {
        center_svr_conn_t free_conn;
        int collect_id = ringbuffer_collect(svr->m_ringbuf);
        if(collect_id < 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: alloc: not enouth capacity, len=%d!",
                center_svr_name(svr), conn->m_fd, (int)size);
            center_svr_conn_free(conn);
            return -1;
        }

        free_conn = center_svr_conn_find_by_fd(svr, collect_id);
        assert(free_conn);

        CPE_INFO(
            svr->m_em, "%s: conn %d: alloc: not enouth free buff, free conn %d!",
            center_svr_name(svr), conn->m_fd, free_conn->m_fd);
        center_svr_conn_free(free_conn);
        if (free_conn == conn) return -1;

        blk = ringbuffer_alloc(svr->m_ringbuf , size);
    }

    *result = blk;
    return 0;
}

uint32_t center_svr_conn_hash(center_svr_conn_t conn) {
    return (uint32_t)conn->m_fd;
}

int center_svr_conn_eq(center_svr_conn_t l, center_svr_conn_t r) {
    return l->m_fd == r->m_fd;
}
