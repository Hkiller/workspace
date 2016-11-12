#include <assert.h>
#include <netdb.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/string_utils.h"
#include "net_dns_task_i.h"

static void net_dns_task_socket_callback(void *data, ares_socket_t socket_fd, int readable, int writable);
static void net_dns_task_dns_callback(void* arg, int status, int timeouts, struct hostent* host);
static void net_dns_task_rw_cb(EV_P_ ev_io *w, int revents);

net_dns_task_t net_dns_ares_gethostbyname(
    net_dns_manage_t mgr, const char * hostname,
    net_dns_process_fun_t process_fun, void * process_ctx)
{
    net_dns_task_t task;
    struct ares_options opts;

    task = mem_alloc(mgr->m_alloc, sizeof(struct net_dns_task));
    if (task == NULL) {
        CPE_ERROR(mgr->m_em, "%s: alloc task fail!", net_dns_manage_name(mgr));
        return NULL;
    }

    task->m_mgr = mgr;
    task->m_process_fun = process_fun;
    task->m_process_ctx = process_ctx;
    ev_init(&task->m_watcher, net_dns_task_rw_cb);
    task->m_watcher.data = task;
    task->m_is_processing = 0;
    task->m_is_destoried = 0;

    opts.sock_state_cb = net_dns_task_socket_callback;
    opts.sock_state_cb_data = task;
    
    if(ares_init_options(&task->m_ares_chanel, &opts, ARES_OPT_SOCK_STATE_CB) != ARES_SUCCESS) {
        CPE_ERROR(mgr->m_em, "%s: : ares init fail", net_dns_manage_name(mgr));
        mem_free(mgr->m_alloc, task);
        return NULL;
    }

    ares_gethostbyname(
        task->m_ares_chanel, hostname, AF_INET,
        net_dns_task_dns_callback, task);
    
    TAILQ_INSERT_TAIL(&mgr->m_tasks, task, m_next);
    return task;
}

void net_dns_task_free(net_dns_task_t task) {
    net_dns_manage_t mgr = task->m_mgr;

    if (task->m_is_processing) {
        assert(!task->m_is_destoried);
        task->m_is_destoried = 1;
        TAILQ_REMOVE(&mgr->m_tasks, task, m_next);
        TAILQ_INSERT_TAIL(&mgr->m_deleting_tasks, task, m_next);

        if (!ev_is_active(&mgr->m_timer_event)) {
            ev_timer_start(mgr->m_ev_loop, &mgr->m_timer_event);
        }
        return;
    }

    ares_destroy(task->m_ares_chanel);
    assert(!ev_is_active(&task->m_watcher));

    if (task->m_is_destoried) {
        TAILQ_REMOVE(&mgr->m_deleting_tasks, task, m_next);
    }
    else {
        TAILQ_REMOVE(&mgr->m_tasks, task, m_next);
    }
    mem_free(mgr->m_alloc, task);
}

void net_dns_task_free_by_ctx(net_dns_manage_t manager, void * process_ctx) {
    net_dns_task_t task, next_task;
    for(task = TAILQ_FIRST(&manager->m_tasks); task; task = next_task) {
        next_task = TAILQ_NEXT(task, m_next);

        if (task->m_process_ctx == process_ctx) {
            net_dns_task_free(task);
        }
    }
}

static void net_dns_task_dns_callback(void* arg, int status, int timeouts, struct hostent* host) {
    net_dns_task_t task = arg;
    net_dns_manage_t mgr = task->m_mgr;

    if (task->m_is_processing) return;

    task->m_is_processing = 1;
    if (status != ARES_SUCCESS) {
        CPE_ERROR(mgr->m_em, "%s: resolved fail (%s)", net_dns_manage_name(mgr), ares_strerror(status));
        task->m_process_fun(task->m_process_ctx, NULL);
    }
    else {
        char ip[64];
        cpe_str_dup(ip, sizeof(ip), inet_ntoa(*((struct in_addr*)host->h_addr)));
        task->m_process_fun(task->m_process_ctx, ip);
    }

    if (!task->m_is_destoried) net_dns_task_free(task);

    task->m_is_processing = 0;
    assert(task->m_is_destoried);
}

static void net_dns_task_rw_cb(EV_P_ ev_io *w, int revents) {
    net_dns_task_t task = w->data;

    assert(task->m_ares_chanel);

    ares_process_fd(task->m_ares_chanel, (revents & EV_READ ? w->fd : ARES_SOCKET_BAD), (revents & EV_WRITE ? w->fd : ARES_SOCKET_BAD));
}

static void net_dns_task_socket_callback(void *data, ares_socket_t socket_fd, int readable, int writable) {
    net_dns_task_t task = data;
    net_dns_manage_t mgr = task->m_mgr;
    
    if (readable || writable) {
        if (ev_is_active(&task->m_watcher)) {
            ev_io_stop(mgr->m_ev_loop, &task->m_watcher);
            CPE_ERROR(mgr->m_em, "%s: %d stop(for restart)", net_dns_manage_name(mgr), socket_fd);
        }
        
        ev_io_set(&task->m_watcher, socket_fd, (readable ? EV_READ : 0) | (writable ? EV_WRITE : 0));
        ev_io_start(mgr->m_ev_loop, &task->m_watcher);
    }
    else {
        if (ev_is_active(&task->m_watcher)) {
            ev_io_stop(mgr->m_ev_loop, &task->m_watcher);
        }
    }
}
