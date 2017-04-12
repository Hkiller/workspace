#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/utils/string_utils.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "net_internal_ops.h"

static void net_connector_cb_clear(net_connector_t connector);
static void net_connector_cb_prepaire(net_connector_t connector);
static void net_connector_on_connected(net_connector_t connector);

#define net_connector_notify_state_change(__c, __o)                 \
    if ((__c)->m_state != (__o)) {                                  \
        struct net_connector_monitor * monitor;                     \
        monitor = (__c)->m_monitors;                                \
        while(monitor) {                                            \
            monitor->m_monitor_fun((__c), monitor->m_monitor_ctx);  \
            monitor = monitor->m_next;                              \
        }                                                           \
    }

net_connector_t
net_connector_create(
    net_mgr_t nmgr,
    const char * name,
    const char * ip,
    short port)
{
    net_connector_t connector;
    char * buf;
    size_t nameLen;
    size_t baseLen;
    baseLen = strlen(name) + 1;

    nameLen = baseLen;
    CPE_PAL_ALIGN_DFT(nameLen);

    buf = mem_alloc(nmgr->m_alloc, sizeof(struct net_connector) + nameLen);
    if (buf == NULL) return NULL;

    memcpy(buf, name, baseLen);
    connector = (net_connector_t)(buf + nameLen);

    connector->m_mgr = nmgr;
    connector->m_name = buf;
    connector->m_state = net_connector_state_disable;
    connector->m_monitors = NULL;
    connector->m_reconnect_span = 30.0;
    connector->m_processing = 0;
    connector->m_deleted = 0;
    connector->m_unregisted = 0;
    connector->m_ep = NULL;
    cpe_hash_entry_init(&connector->m_hh);

    if (cpe_hash_table_insert_unique(&nmgr->m_connectors, connector) != 0) {
        CPE_ERROR(nmgr->m_em, "connector %s is already exist!", name);
        mem_free(nmgr->m_alloc, buf);
        return NULL;
    }

    net_connector_set_address(connector, ip, port);

    return connector;
}

void net_connector_free(net_connector_t connector) {
    struct net_connector_monitor ** monitor;
    net_ep_t ep;

    assert(connector);
    assert(connector->m_mgr);

    if (connector->m_unregisted == 0) {
        cpe_hash_table_remove_by_ins(&connector->m_mgr->m_connectors, connector);
        connector->m_unregisted = 1;
    }

    if (connector->m_ep) {
        ep = connector->m_ep;
        net_connector_unbind(ep->m_connector);
        net_ep_free(ep);
    }

    monitor = &connector->m_monitors;
    while(*monitor) {
        struct net_connector_monitor * cur = *monitor;
        *monitor = cur->m_next;
        cur->m_next = NULL;
        mem_free(connector->m_mgr->m_alloc, cur);
    }

    if (connector->m_processing) {
        assert(!connector->m_deleted);
        connector->m_deleted = 1;
    }
    else {
        mem_free(connector->m_mgr->m_alloc, (void*)connector->m_name);
    }
}

int net_connector_set_address(net_connector_t connector, const char * ip, short port) {
    struct sockaddr_in * inetAddr;

    if (connector->m_state != net_connector_state_disable) return -1;

    if (ip && ip[0] != 0) {
        inetAddr = (struct sockaddr_in *)(&connector->m_addr);
        inetAddr->sin_family = AF_INET;
        inetAddr->sin_port = htons(port);
        inetAddr->sin_addr.s_addr = inet_addr(ip);

        cpe_str_dup(connector->m_ip, sizeof(connector->m_ip), ip);
        connector->m_port = port;
    }
    else {
        connector->m_ip[0] = 0;
        connector->m_port = port;
    }

    return 0;
}

const char * net_connector_ip(net_connector_t connector) {
    return connector->m_ip;
}

short net_connector_port(net_connector_t connector) {
    return connector->m_port;
}

net_connector_t
net_connector_create_with_ep(
    net_mgr_t nmgr,
    const char * name,
    const char * ip,
    short port)
{
    net_connector_t connector;
    net_ep_t ep;

    connector = net_connector_create(nmgr, name, ip, port);
    if (connector == NULL) return NULL;

    ep = net_ep_create(nmgr);
    if (ep == NULL) {
        net_connector_free(connector);
        return NULL;
    }

    if (net_connector_bind(connector, ep) != 0) {
        net_connector_free(connector);
        net_ep_free(ep);
        return NULL;
    }

    return connector;
}

net_connector_t
net_connector_find(net_mgr_t nmgr, const char * name) {
    struct net_connector key;

    key.m_name = name;
    return (net_connector_t)cpe_hash_table_find(&nmgr->m_connectors, &key);
}

const char * net_connector_name(net_connector_t connector) {
    return connector->m_name;
}

net_connector_state_t net_connector_state(net_connector_t connector) {
    return connector->m_state;
}

int net_connector_add_monitor(
    net_connector_t connector,
    net_connector_state_monitor_fun_t fun, void * ctx)
{
    struct net_connector_monitor * new_monitor;
    struct net_connector_monitor * * monitors = &connector->m_monitors;

    while(*monitors) {
        if ((*monitors)->m_monitor_fun == fun && (*monitors)->m_monitor_ctx == ctx) {
            return 0;
        }
        monitors = &(*monitors)->m_next;
    }

    new_monitor = (struct net_connector_monitor *)mem_alloc(connector->m_mgr->m_alloc, sizeof(struct net_connector_monitor));
    if (new_monitor == NULL) return -1;
 
    new_monitor->m_monitor_fun = fun;
    new_monitor->m_monitor_ctx = ctx;
    new_monitor->m_next = NULL;

    *monitors = new_monitor;

    return 0;
}

int net_connector_remove_monitor(
    net_connector_t connector,
    net_connector_state_monitor_fun_t fun, void * ctx)
{
    int rv;
    struct net_connector_monitor * * monitors = &connector->m_monitors;

    rv = 0;
    while(*monitors) {
        struct net_connector_monitor * cur = *monitors;
        
        if (cur->m_monitor_fun == fun && cur->m_monitor_ctx == ctx) {
            *monitors = cur->m_next;
            mem_free(connector->m_mgr->m_alloc, cur);
            ++rv;
        }
        else {
            monitors = &cur->m_next;
        }
    }

    return rv;
}

int net_connector_bind(net_connector_t connector, net_ep_t ep) {
    if (connector->m_ep) {
        CPE_ERROR(connector->m_mgr->m_em, "connector %s already have ep!", connector->m_name);
        return -1;
    }

    if (ep->m_connector) {
        CPE_ERROR(connector->m_mgr->m_em, "endpoint %d already have connector!", ep->m_id);
        return -1;
    }

    if (ep->m_connector) {
        CPE_ERROR(connector->m_mgr->m_em, "endpoint %d already have connector!", ep->m_id);
        return -1;
    }

    connector->m_ep = ep;
    ep->m_connector = connector;

    if (net_ep_is_open(connector->m_ep)) {
        net_ep_close_i(connector->m_ep, net_ep_event_close_by_error);
    }

    return 0;
}

int net_connector_unbind(net_connector_t connector) {
    if (connector->m_ep == NULL) return 0;

    assert(connector->m_ep->m_connector == connector);

    net_connector_cb_clear(connector);

    connector->m_ep->m_connector = NULL;
    connector->m_ep = NULL;
    connector->m_state = net_connector_state_disable;

    return 0;
}

void net_connector_on_disconnect(net_connector_t connector) {
    net_connector_state_t old_state;

    old_state = connector->m_state;

    net_connector_cb_clear(connector);

    connector->m_state = net_connector_state_error;

    net_connector_cb_prepaire(connector);

    net_connector_notify_state_change(connector, old_state);

}

uint32_t net_connector_hash(net_connector_t connector) {
    return cpe_hash_str(connector->m_name, strlen(connector->m_name));
}

int net_connector_cmp(net_connector_t l, net_connector_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void net_connectors_free(net_mgr_t nmgr) {
    struct cpe_hash_it hashIt;
    net_connector_t pre;
    net_connector_t cur;

    cpe_hash_it_init(&hashIt, &nmgr->m_connectors);

    pre = NULL;
    while((cur = (net_connector_t)cpe_hash_it_next(&hashIt))) {
        if (pre) net_connector_free(pre);
        pre = cur;
    }

    if (pre) net_connector_free(pre);
}

static void net_connector_do_connect_i(net_connector_t connector) {
    net_ep_t ep;

    assert(connector);

    ep = connector->m_ep;
    assert(ep);

    ep->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, 0);
    if (ep->m_fd == -1) {
        CPE_ERROR(
            connector->m_mgr->m_em,
            "connector %s: create socket fail, errno=%d (%s)!",
            connector->m_name, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        connector->m_state = net_connector_state_error;
        return;
    }

    if (net_socket_set_none_block(ep->m_fd, connector->m_mgr->m_em)) {
        net_socket_close(&ep->m_fd, connector->m_mgr->m_em);
        connector->m_state = net_connector_state_error;
        return;
    }

    if (cpe_connect(ep->m_fd, (struct sockaddr *)&connector->m_addr, sizeof(connector->m_addr)) == -1) {
#ifdef _MSC_VER
        if (cpe_sock_errno() == WSAEWOULDBLOCK) {
#else
        if (cpe_sock_errno() == EINPROGRESS) {
#endif
            CPE_INFO(
                connector->m_mgr->m_em,
                "connector %s: connecting!",
                connector->m_name);
            connector->m_state = net_connector_state_connecting;
            return;
        }
        else {
            CPE_ERROR(
                connector->m_mgr->m_em,
                "connector %s: connect error, errno=%d (%s)",
                connector->m_name, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            net_socket_close(&ep->m_fd, connector->m_mgr->m_em);
            connector->m_state = net_connector_state_error;
            return;
        }
    }
    else {
        CPE_INFO(
            connector->m_mgr->m_em,
            "connector %s: connected success!",
            connector->m_name);
        connector->m_state = net_connector_state_connected;
        return;
    }
};

static void net_connector_on_connected(net_connector_t connector) {
    int fd = connector->m_ep->m_fd;
    connector->m_ep->m_fd = -1;
    net_ep_set_fd(connector->m_ep, fd);
}

static void net_connector_check_connect_result(net_connector_t connector) {
    int err;
    socklen_t err_len;

    err_len = sizeof(err);

    if (cpe_getsockopt(connector->m_ep->m_fd, SOL_SOCKET, SO_ERROR, (void*)&err, &err_len) == -1) {
        CPE_ERROR(
            connector->m_mgr->m_em,
            "connector %s: check state, getsockopt error, errno=%d (%s)",
            connector->m_name, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        net_socket_close(&connector->m_ep->m_fd, connector->m_mgr->m_em);
        connector->m_state = net_connector_state_error;
    }
    else {
        if (err == 0) {
            CPE_INFO(
                connector->m_mgr->m_em,
                "connector %s: connected success!",
                connector->m_name);
            connector->m_state = net_connector_state_connected;
        }
        else {
            CPE_ERROR(
                connector->m_mgr->m_em,
                "connector %s: connect error, errno=%d (%s)",
                connector->m_name, err, cpe_sock_errstr(err));
            net_socket_close(&connector->m_ep->m_fd, connector->m_mgr->m_em);
            connector->m_state = net_connector_state_error;
        }
    }
}

static void net_connector_io_cb_connect(EV_P_ ev_io *w, int revents) {
    net_connector_state_t old_state;
    net_connector_t connector;

    ev_io_stop(EV_A_ w);

    connector = (net_connector_t)w->data;
    assert(connector);

    old_state = connector->m_state;

    connector->m_processing = 1;

    net_connector_check_connect_result(connector);

    if (!connector->m_deleted) {
        if (connector->m_state == net_connector_state_connected) {
            net_connector_on_connected(connector);
        }
        else {
            net_connector_cb_prepaire(connector);
        }
    }

    if (!connector->m_deleted) {
        net_connector_notify_state_change(connector, old_state);
    }

    connector->m_processing = 0;

    if (connector->m_deleted) {
        net_connector_free(connector);
    }
}

static void net_connector_timer_cb_reconnect(EV_P_ ev_timer *w, int revents) {
    net_connector_state_t old_state;
    net_connector_t connector;

    ev_timer_stop(EV_A_ w);
    
    connector = (net_connector_t)w->data;
    assert(connector);

    old_state = connector->m_state;
    connector->m_state = net_connector_state_idle;
    
    connector->m_processing = 1;

    if (!connector->m_deleted) {
        net_connector_do_connect_i(connector);
    }

    if (!connector->m_deleted) {
        if (connector->m_state == net_connector_state_connected) {
            net_connector_on_connected(connector);
        }
        else {
            net_connector_cb_prepaire(connector);
        }
    }

    connector->m_processing = 0;

    if (!connector->m_deleted) {
        net_connector_notify_state_change(connector, old_state);
    }
}

static void net_connector_cb_clear(net_connector_t connector) {
    if (connector->m_state == net_connector_state_connecting) {
        ev_io_stop(connector->m_mgr->m_ev_loop, &connector->m_ep->m_watcher);
        connector->m_state = net_connector_state_idle;
    }
    else if (connector->m_state == net_connector_state_error) {
        ev_timer_stop(connector->m_mgr->m_ev_loop, &connector->m_timer);
        connector->m_state = net_connector_state_idle;
    }

    if (connector->m_deleted) {
        net_connector_free(connector);
    }
}

static void net_connector_cb_prepaire(net_connector_t connector) {
    if (connector->m_state == net_connector_state_connecting) {
        connector->m_ep->m_watcher.data = connector;
        ev_io_init(&connector->m_ep->m_watcher, net_connector_io_cb_connect, connector->m_ep->m_fd, EV_WRITE);
        ev_io_start(connector->m_mgr->m_ev_loop, &connector->m_ep->m_watcher);
    }
    else if (connector->m_state == net_connector_state_error) {
        connector->m_timer.data = connector;
        ev_timer_init (&connector->m_timer, net_connector_timer_cb_reconnect, connector->m_reconnect_span, 0.);
        ev_timer_start(connector->m_mgr->m_ev_loop, &connector->m_timer);
    }
}

static void net_connector_do_connect(net_connector_t connector) {
    net_connector_state_t old_state;

    assert(connector->m_state != net_connector_state_connected);

    old_state = connector->m_state;

    net_connector_cb_clear(connector);
    net_connector_do_connect_i(connector);

    if (connector->m_state == net_connector_state_connected) {
        net_connector_on_connected(connector);
    }
    else {
        net_connector_cb_prepaire(connector);
    }

    connector->m_processing = 1;
    net_connector_notify_state_change(connector, old_state);
    connector->m_processing = 0;

    if (connector->m_deleted) {
        net_connector_free(connector);
    }
}

int net_connector_enable(net_connector_t connector) {
    assert(connector);
    if (connector->m_ep == NULL) {
        CPE_ERROR(
            connector->m_mgr->m_em,
            "connector %s: can`t enable for no ep binded!",
            connector->m_name);
        return -1;
    }

    if (connector->m_ip[0] == 0) {
        CPE_ERROR(connector->m_mgr->m_em, "connector %s: address not set!", connector->m_name);
        return -1;
    }

    if (connector->m_state != net_connector_state_disable) {
        CPE_INFO(
            connector->m_mgr->m_em,
            "connector %s: already enable!",
            connector->m_name);
        return 0;
    }

    connector->m_state = net_connector_state_idle;
    net_connector_do_connect(connector);

    return 0;
}

void net_connector_disable(net_connector_t connector) {
    net_connector_state_t old_state;

    if (connector->m_state == net_connector_state_disable) return;

    assert(connector->m_ep);
    old_state = connector->m_state;

    net_connector_cb_clear(connector);

    if (net_ep_is_open(connector->m_ep)) {
        net_ep_close_i(connector->m_ep, net_ep_event_close_by_user);
    }

    connector->m_state = net_connector_state_disable;

    net_connector_notify_state_change(connector, old_state);
}

void net_connector_set_reconnect_span_ms(net_connector_t connector, uint64_t span_ms) {
    connector->m_reconnect_span = ((double)span_ms / 1000.0);
}

net_ep_t net_connector_ep(net_connector_t connector) {
    return connector->m_ep;
}

const char * net_connector_state_str(net_connector_state_t state) {
    switch(state) {
    case net_connector_state_disable:
        return "net_connector_state_disable";
    case net_connector_state_idle:
        return "net_connector_state_idle";
    case net_connector_state_connecting:
        return "net_connector_state_connecting";
    case net_connector_state_connected:
        return "net_connector_state_connected";
    case net_connector_state_error:
        return "net_connector_state_error";
    default:
        return "net_connector_state_unknown";
    }
}
