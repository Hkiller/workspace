#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_connector.h"
#include "net_internal_ops.h"

static void net_ep_cb(EV_P_ ev_io *w, int revents);
static void net_ep_timeout_cb(EV_P_ ev_timer *w, int revents);

net_ep_t
net_ep_create(net_mgr_t nmgr) {
    net_ep_t ep;

    assert(nmgr);

    ep = net_ep_pages_alloc_ep(nmgr);
    if (ep == NULL) return NULL;

    assert(ep->m_id != GD_NET_EP_INVALID_ID);

    ep->m_mgr = nmgr;
    ep->m_chanel_r = NULL;
    ep->m_chanel_w = NULL;
    ep->m_connector = NULL;
    ep->m_fd = -1;
	ep->m_status = NET_INVALID;
    ep->m_process_fun = NULL;
    ep->m_process_ctx = NULL;
    ep->m_processing = 0;
    ep->m_deleted = 0;

    ep->m_watcher.data = ep;
    ev_init(&ep->m_watcher, net_ep_cb);

    ep->m_timer.data = ep;
    ev_init(&ep->m_timer, NULL);
    ep->m_timer.repeat = 0;

    return ep;
}

void net_ep_free(net_ep_t ep) {
    if (ep->m_connector) {
        net_connector_unbind(ep->m_connector);
        assert(ep->m_connector == NULL);
    }

    if (ev_cb(&ep->m_timer) == net_ep_timeout_cb) {
        ev_timer_stop(ep->m_mgr->m_ev_loop, &ep->m_timer);
        ev_init(&ep->m_timer, NULL);
        ev_timer_set(&ep->m_timer, 0, 0);
    }

    if (net_ep_is_open(ep)) {
        net_ep_close_i(ep, net_ep_event_close_by_shutdown);
    }

    if (ep->m_processing) {
        assert(!ep->m_deleted);
        ep->m_deleted = 1;
    }
    else {
        if (ep->m_chanel_w) {
            net_chanel_free(ep->m_chanel_w);
            ep->m_chanel_w = NULL;
        }

        if (ep->m_chanel_r) {
            net_chanel_free(ep->m_chanel_r);
            ep->m_chanel_r = NULL;
        }

        net_ep_pages_free_ep(ep);
    }
}

net_ep_id_t net_ep_id(net_ep_t ep) {
    return ep->m_id;
}

net_mgr_t net_ep_mgr(net_ep_t ep) {
    return ep->m_mgr;
}

void net_ep_set_processor(net_ep_t ep, net_process_fun_t process_fun, void * process_ctx) {
    ep->m_process_fun = process_fun;
    ep->m_process_ctx = process_ctx;

    if (process_fun && net_ep_is_open(ep)) {
        process_fun(ep, process_ctx, net_ep_event_open);
    }
}

#define net_ep_calc_ev_events(ep)                               \
    (((ep->m_chanel_r                                           \
       && ep->m_chanel_r->m_state != net_chanel_state_full)     \
      ? EV_READ : 0)                                            \
     |                                                          \
     ((ep->m_chanel_w                                           \
       && ep->m_chanel_w->m_state != net_chanel_state_empty)    \
      ? EV_WRITE : 0))

#define net_ep_update_events(ep, old_events)                        \
    if (ep->m_fd >= 0) {                                            \
        int new_events = net_ep_calc_ev_events(ep);                 \
        if (old_events != new_events) {                             \
            ev_io_stop(ep->m_mgr->m_ev_loop, &ep->m_watcher);       \
            if (new_events) {                                       \
                ep->m_watcher.data = ep;                            \
                ev_io_set(&ep->m_watcher, ep->m_fd, new_events);    \
                ev_io_start(ep->m_mgr->m_ev_loop, &ep->m_watcher);  \
            }                                                       \
        }                                                           \
    } while(0)

net_chanel_t
net_ep_chanel_r(net_ep_t ep) {
    return ep->m_chanel_r;
}

void net_ep_set_chanel_r(net_ep_t ep, net_chanel_t chanel) {
    int old_events;

    old_events = net_ep_calc_ev_events(ep);

    if (ep->m_chanel_r) {
        net_chanel_free(ep->m_chanel_r);
    }

    ep->m_chanel_r = chanel;

    net_ep_update_events(ep, old_events);
}

net_chanel_t
net_ep_chanel_w(net_ep_t ep) {
    return ep->m_chanel_w;
}

void net_ep_set_chanel_w(net_ep_t ep, net_chanel_t chanel) {
    int old_events;

    old_events = net_ep_calc_ev_events(ep);

    if (ep->m_chanel_w) {
        net_chanel_free(ep->m_chanel_w);
    }

    ep->m_chanel_w = chanel;

    net_ep_update_events(ep, old_events);
}

int net_ep_is_open(net_ep_t ep) {
    return ep->m_fd < 0 ? 0 : 1;
}

int net_ep_set_timeout(net_ep_t ep, tl_time_span_t span) {
    if (span == 0) {
        if (ev_cb(&ep->m_timer) == NULL) {
            return 0;
        }
        else if (ev_cb(&ep->m_timer) == net_ep_timeout_cb) {
            ev_timer_stop(ep->m_mgr->m_ev_loop, &ep->m_timer);
            ev_init(&ep->m_timer, NULL);
            ev_timer_set(&ep->m_timer, 0, 0);
            return 0;
        }
        else {
            return -1;
        }
    }
    else {
        ev_tstamp ev_span = (ev_tstamp)span / 1000.0;

        if (ev_cb(&ep->m_timer) == NULL) {
            ev_timer_init(&ep->m_timer, net_ep_timeout_cb, ev_span, ev_span);
            ev_timer_start(ep->m_mgr->m_ev_loop, &ep->m_timer);
            return 0;
        }
        else if (ev_cb(&ep->m_timer) == net_ep_timeout_cb) {
            ev_timer_stop(ep->m_mgr->m_ev_loop, &ep->m_timer);
            ev_timer_set(&ep->m_timer, ev_span, ev_span);
            ev_timer_start(ep->m_mgr->m_ev_loop, &ep->m_timer);
            return 0;
        }
        else {
            return -1;
        }
    }
}

tl_time_span_t net_ep_timeout(net_ep_t ep) {
    return ev_cb(&ep->m_timer) == net_ep_timeout_cb
        ? (tl_time_span_t)(ep->m_timer.repeat * 1000)
        : 0;
}

void net_ep_close_i(net_ep_t ep, net_ep_event_t ev) {
    int fd;

    assert(ep);
    assert(ev == net_ep_event_close_by_user
           || ev == net_ep_event_close_by_peer
           || ev == net_ep_event_close_by_error
           || ev == net_ep_event_close_by_shutdown);

    if (ep->m_fd < 0) return;

    fd = ep->m_fd;
    ep->m_fd = -1;

    ev_io_stop(ep->m_mgr->m_ev_loop, &ep->m_watcher);

    net_socket_close(&fd, ep->m_mgr->m_em);
    net_ep_set_status(ep, NET_INVALID);

    if (ep->m_connector) {
        net_connector_on_disconnect(ep->m_connector);
    }

    if (ep->m_process_fun) ep->m_process_fun(ep, ep->m_process_ctx, ev);
}

void net_ep_close(net_ep_t ep) {
    net_ep_close_i(ep, net_ep_event_close_by_user);
}

size_t net_ep_size(net_ep_t ep) {
    assert(ep);
    return ep->m_chanel_r ? ep->m_chanel_r->m_type->data_size(ep->m_chanel_r) : 0;
}

int net_ep_localname(net_ep_t ep, uint32_t * ip, uint16_t * port) {
    struct sockaddr_in addr;
    socklen_t len;

    if (ep->m_fd < 0) return -1;

    len = sizeof(addr);
    if (cpe_getsockname(ep->m_fd, (struct sockaddr *)&addr, &len) != 0) {
        CPE_ERROR(
            ep->m_mgr->m_em,
            "getsockname fail, errno=%d (%s)!",
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }

    if (port) *port = ntohs(addr.sin_port);
    if (ip) *ip = addr.sin_addr.s_addr;

    return 0;
}

int net_ep_peername(net_ep_t ep, uint32_t * ip, uint16_t * port) {
    struct sockaddr_in addr;
    socklen_t len;

    if (ep->m_fd < 0) return -1;

    len = sizeof(addr);
    if (cpe_getpeername(ep->m_fd, (struct sockaddr *)&addr, &len) != 0) {
        CPE_ERROR(
            ep->m_mgr->m_em,
            "getsockname fail, errno=%d (%s)!",
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }

    if (port) *port = ntohs(addr.sin_port);
    if (ip) *ip = addr.sin_addr.s_addr;

    return 0;
}


int net_ep_set_fd(net_ep_t ep, int fd) {
    assert(ep);

    if (net_ep_is_open(ep)) {
        CPE_ERROR(
            ep->m_mgr->m_em, "net_mgr: ep %d: set fd to a opend ep",
            (int)ep->m_id);
        net_ep_close_i(ep, net_ep_event_close_by_error);
    }

    ep->m_fd = fd;

    ep->m_watcher.data = ep;
    ev_init(&ep->m_watcher, net_ep_cb);

    net_ep_update_events(ep, 0);

    return 0;
}

void net_ep_set_status(net_ep_t ep, enum net_status status) {
    assert(ep);
    ep->m_status = status;
}

int net_ep_send(net_ep_t ep, const void * buf, size_t size) {
    int old_events;
    int r;

    assert(ep);
    if (buf == NULL || size <= 0) return -1;
    if (ep->m_chanel_w == NULL) return -1;

    old_events = net_ep_calc_ev_events(ep);

    r = ep->m_chanel_w->m_type->read_from_buf(ep->m_chanel_w, buf, size);

    net_ep_update_events(ep, old_events);

    return r;
}

ssize_t net_ep_rece(net_ep_t ep, void * buf, size_t capacity) {
    int old_events;
    ssize_t r;

    assert(ep);
    if (buf == NULL) return -1;
    if (ep->m_chanel_r == NULL) return -1;

    old_events = net_ep_calc_ev_events(ep);

    r = ep->m_chanel_r->m_type->write_to_buf(ep->m_chanel_r, buf, capacity);

    net_ep_update_events(ep, old_events);

    return r;
}

void * net_ep_peek(net_ep_t ep, void * buf, size_t size) {
    assert(ep);
    if (ep->m_chanel_r == NULL) return NULL;

    return ep->m_chanel_r->m_type->peek(ep->m_chanel_r, buf, size);
}

void net_ep_erase(net_ep_t ep, size_t size) {
    int old_events;

    assert(ep);
    if (ep->m_chanel_r == NULL) return;

    old_events = net_ep_calc_ev_events(ep);

    ep->m_chanel_r->m_type->erase(ep->m_chanel_r, size);

    net_ep_update_events(ep, old_events);
}

void net_ep_timeout_cb(EV_P_ ev_timer *w, int revents) {
    net_ep_t ep  = (net_ep_t)w->data;
    assert(ep);

    CPE_ERROR(
        ep->m_mgr->m_em,
        "net_mgr: ep %d: connection timeout!",
        ep->m_id);

    net_ep_close(ep);
}

void net_ep_cb(EV_P_ ev_io *w, int revents) {
    net_ep_t ep;
    int old_events;

    ep = (net_ep_t)w->data;
    assert(ep);
    assert(ep->m_processing == 0);
    assert(!ep->m_deleted);

    ep->m_processing = 1;

    old_events = net_ep_calc_ev_events(ep);

    if (!ep->m_deleted && revents & EV_READ) {
        ssize_t recv_size = ep->m_chanel_r->m_type->read_from_net(ep->m_chanel_r, ep->m_fd);
        if (recv_size < 0) {
            CPE_ERROR(
                ep->m_mgr->m_em,
                "net_mgr: ep %d: read data error, errno=%d (%s)",
                ep->m_id, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            net_ep_close_i(ep, net_ep_event_close_by_error);
            ep->m_processing = 0;
            return;
        }
        else if (recv_size == 0) {
            if (ep->m_mgr->m_debug) {
                CPE_INFO(ep->m_mgr->m_em, "net_mgr: ep %d: socket close by peer!", ep->m_id);
            }
            net_ep_close_i(ep, net_ep_event_close_by_peer);
            ep->m_processing = 0;
            return;
        }
        else {
            if (ep->m_mgr->m_debug) {
                CPE_INFO(ep->m_mgr->m_em, "net_mgr: ep %d: receive %d types data!", ep->m_id, (int)recv_size);
            }

            if (ep->m_process_fun) {
                ep->m_process_fun(ep, ep->m_process_ctx, net_ep_event_read);
            }
        }
    }

    if (!ep->m_deleted && revents & EV_WRITE) {
        ssize_t send_size = ep->m_chanel_w->m_type->write_to_net(ep->m_chanel_w, ep->m_fd);
        if (send_size < 0) {
            CPE_ERROR(
                ep->m_mgr->m_em,
                "net_mgr: ep %d: write data error, errno=%d (%s)",
                ep->m_id, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            net_ep_close_i(ep, net_ep_event_close_by_error);
            ep->m_processing = 0;
            return;
        }
        else {
            if (ep->m_mgr->m_debug) {
                CPE_INFO(ep->m_mgr->m_em, "net_mgr: ep %d: send %d bytes data!", ep->m_id, (int)send_size);
            }

			if(ep->m_status == NET_REMOVE_AFTER_SEND) {
                if (ep->m_chanel_w->m_type->data_size(ep->m_chanel_w) == 0) {
                    if (ep->m_mgr->m_debug) {
                        CPE_INFO(ep->m_mgr->m_em, "net_mgr: ep %d: write chanel empty, auto close!", ep->m_id);
                    }
                    net_ep_close_i(ep, net_ep_event_close_by_user);
                    ep->m_processing = 0;
                    return;
                }
			}
        }
    }

    if (!ep->m_deleted && ev_cb(&ep->m_timer) == net_ep_timeout_cb) {
        ev_tstamp span = ep->m_timer.repeat;

        assert(span > 0);

        ev_timer_stop(ep->m_mgr->m_ev_loop, &ep->m_timer);
        ev_timer_set(&ep->m_timer, span, span);
        ev_timer_start(ep->m_mgr->m_ev_loop, &ep->m_timer);
    }

    ep->m_processing = 0;
    net_ep_update_events(ep, old_events);

    if (ep->m_deleted) net_ep_free(ep);
}

const char * net_ep_event_str(net_ep_event_t evt) {
    switch(evt) {
    case net_ep_event_read:
        return "net_ep_event_read";
    case net_ep_event_open:
        return "net_ep_event_open";
    case net_ep_event_close_by_user:
        return "net_ep_event_close_by_user";
    case net_ep_event_close_by_peer:
        return "net_ep_event_close_by_peer";
    case net_ep_event_close_by_error:
        return "net_ep_event_close_by_error";
    case net_ep_event_close_by_shutdown:
        return "net_ep_event_close_by_shutdown";
    default:
        return "net_ep_event_unknown";
    }
}
