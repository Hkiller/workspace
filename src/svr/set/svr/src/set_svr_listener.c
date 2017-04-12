#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "set_svr_listener.h"
#include "set_svr_set_conn.h"

set_svr_listener_t set_svr_listener_create(set_svr_t svr) {
    set_svr_listener_t listener;

    listener = mem_alloc(svr->m_alloc, sizeof(struct set_svr_listener));
    if (listener == NULL) {
        CPE_ERROR(listener->m_svr->m_em, "%s: listener: create: alloc fail!", set_svr_name(listener->m_svr));
        return NULL;
    }

    listener->m_svr = svr;
    listener->m_ip[0] = 0;
    listener->m_port = 0;
    listener->m_fd = -1;

    return listener;
}

void set_svr_listener_free(set_svr_listener_t listener) {
    if (listener->m_fd >= 0) {
        set_svr_listener_stop(listener);
    }
}

void set_svr_listener_cb(EV_P_ ev_io *w, int revents) {
    set_svr_listener_t listener;
    int new_fd;
    set_svr_set_conn_t conn;
    
    listener = w->data;
    assert(listener);

    new_fd = cpe_accept(listener->m_fd, 0, 0);
    if (new_fd == -1) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: listener: accept error, errno=%d (%s)",
            set_svr_name(listener->m_svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return;
    }

    conn = set_svr_set_conn_create(listener->m_svr, NULL, new_fd);
    if (conn == NULL) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: listener: crate conn fail",
            set_svr_name(listener->m_svr));
        cpe_sock_close(new_fd);
        return;
    }
}

int set_svr_listener_start(set_svr_listener_t listener, const char * ip, uint16_t port, int accept_queue_size) {
    struct sockaddr_in addr;

    assert(listener->m_fd < 0);
    
    listener->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener->m_fd == -1) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: listener: socket call fail, errno=%d (%s)!",
            set_svr_name(listener->m_svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }

    if (cpe_sock_set_reuseaddr(listener->m_fd, 1) != 0) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: listener: set sock reuseaddr fail, errno=%d (%s)!",
            set_svr_name(listener->m_svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(listener->m_fd);
        listener->m_fd = -1;
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(cpe_bind(listener->m_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: listener: bind address %s.%d error, errno=%d (%s)",
            set_svr_name(listener->m_svr), ip, port, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(listener->m_fd);
        listener->m_fd = -1;
        return -1;
    }

    if (accept_queue_size == 0) accept_queue_size = 512;

    if (cpe_listen(listener->m_fd, accept_queue_size) != 0) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: listener: listen error, errno=%d (%s)",
            set_svr_name(listener->m_svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(listener->m_fd);
        listener->m_fd = -1;
        return -1;
    }

    listener->m_watcher.data = listener;
    ev_io_init(&listener->m_watcher, set_svr_listener_cb, listener->m_fd, EV_READ);
    ev_io_start(listener->m_svr->m_ev_loop, &listener->m_watcher);

    cpe_str_dup(listener->m_ip, sizeof(listener->m_ip), ip);
    listener->m_port = port;
    
    if (listener->m_svr->m_debug) {
        CPE_INFO(listener->m_svr->m_em, "%s: listener: listen at %s:%d start", set_svr_name(listener->m_svr), listener->m_ip, listener->m_port);
    }

    return 0;
}

void set_svr_listener_stop(set_svr_listener_t listener) {
    if (listener->m_fd < 0) return;

    ev_io_stop(listener->m_svr->m_ev_loop, &listener->m_watcher);
    cpe_sock_close(listener->m_fd);
    listener->m_fd = -1;

    if (listener->m_svr->m_debug) {
        CPE_INFO(listener->m_svr->m_em, "%s: listener: listen stop", set_svr_name(listener->m_svr));
    }
}
