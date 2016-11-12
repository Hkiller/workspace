#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "net_trans_manage_i.h"
#include "net_trans_task_i.h"

static int net_trans_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
static int net_trans_timer_cb(CURLM *multi, long timeout_ms, net_trans_manage_t mgr);
static void net_trans_event_cb(EV_P_ struct ev_io *w, int revents);
static void net_trans_check_multi_info(net_trans_manage_t mgr);

int net_trans_mult_handler_init(net_trans_manage_t mgr) {
    assert(mgr->m_multi_handle == NULL);

	mgr->m_multi_handle = curl_multi_init();
	if (mgr->m_multi_handle == NULL) {
		CPE_ERROR(
            mgr->m_em, "%s: create: curl_multi_init error, errno=%d, %s",
            net_trans_manage_name(mgr), errno, strerror(errno));
		return -1;
	}

    curl_multi_setopt(mgr->m_multi_handle, CURLMOPT_SOCKETFUNCTION, net_trans_sock_cb);
    curl_multi_setopt(mgr->m_multi_handle, CURLMOPT_SOCKETDATA, mgr);
    curl_multi_setopt(mgr->m_multi_handle, CURLMOPT_TIMERFUNCTION, net_trans_timer_cb);
    curl_multi_setopt(mgr->m_multi_handle, CURLMOPT_TIMERDATA, mgr);

    return 0;
}

void net_trans_mult_handler_fini(net_trans_manage_t mgr) {
    assert(mgr->m_multi_handle);
    curl_multi_cleanup(mgr->m_multi_handle);
}

static int net_trans_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
    net_trans_manage_t mgr = cbp;
    net_trans_task_t task;

    curl_easy_getinfo(e, CURLINFO_PRIVATE, &task);

    assert(mgr);
    assert(task);

    if (mgr->m_debug >= 2) {
        static char * what_msgs[] = { "NONE", "IN", "OUT", "INOUT", "REMOVE" };
        CPE_INFO(
            mgr->m_em, "%s: sock_cb: task %d (%s) %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, what_msgs[what]);
    }

    if (what == CURL_POLL_REMOVE) {
        task->m_sockfd = -1;
        if (task->m_evset) {
            ev_io_stop(mgr->m_loop, &task->m_watch);
            task->m_evset = 0;
        }
    }
    else {
        int kind =
            (what & CURL_POLL_IN ? EV_READ : 0)
            | (what & CURL_POLL_OUT ? EV_WRITE : 0);

        task->m_sockfd = s;
        if (task->m_evset) {
            ev_io_stop(mgr->m_loop, &task->m_watch);
        }
#ifdef _WIN32
        ev_io_init(&task->m_watch, net_trans_event_cb, _open_osfhandle(task->m_sockfd, 0) , kind);
#else
        ev_io_init(&task->m_watch, net_trans_event_cb, task->m_sockfd, kind);
#endif
        ev_io_start(mgr->m_loop, &task->m_watch);

        task->m_evset = 1;
    }

    return 0;
}

static void net_trans_event_cb(EV_P_ struct ev_io *w, int revents) {
    net_trans_task_t task = w->data;
    net_trans_manage_t mgr = task->m_group->m_mgr;
    int action;
    int rc;

    if (mgr->m_debug >= 2) {
        CPE_INFO(
            mgr->m_em, "%s: socket event%s%s", net_trans_manage_name(mgr),
            (revents & EV_READ ? " read" : ""),
            (revents & EV_WRITE ? " write" : ""));
    }

    action =
        (revents & EV_READ ? CURL_POLL_IN : 0)
        | (revents & EV_WRITE ? CURL_POLL_OUT : 0);

    rc = curl_multi_socket_action(mgr->m_multi_handle, task->m_sockfd, action, &mgr->m_still_running);
    if (rc != CURLM_OK) {
        CPE_ERROR(mgr->m_em, "%s: event_cb: curl_multi_socket_action return error %d!", net_trans_manage_name(mgr), rc);
        net_trans_task_set_done(task, net_trans_result_error, -1);
        return;
    }

    net_trans_check_multi_info(mgr);

    if (mgr->m_still_running <= 0) {
        if(mgr->m_debug) {
            CPE_INFO(mgr->m_em, "%s: event_cb: last transfer done, kill timeout", net_trans_manage_name(mgr));
        }
        ev_timer_stop(mgr->m_loop, &mgr->m_timer_event);
    }
}

static void net_trans_check_multi_info(net_trans_manage_t mgr) {
    CURLMsg *msg;
    int msgs_left;
    CURL * handler;
    CURLcode res;
    net_trans_task_t task;

    while ((msg = curl_multi_info_read(mgr->m_multi_handle, &msgs_left))) {
        switch(msg->msg) {
        case CURLMSG_DONE: {
            handler = msg->easy_handle;
            res = msg->data.result;

            curl_easy_getinfo(handler, CURLINFO_PRIVATE, &task);

            switch(res) {
            case CURLE_OK:
                net_trans_task_set_done(task, net_trans_result_ok, 0);
                break;
            case CURLE_OPERATION_TIMEDOUT:
                net_trans_task_set_done(task, net_trans_result_timeout, -1);
                break;
            default:
                CPE_ERROR(
                    mgr->m_em, "%s: task %d (%s): check_multi_info done: res=%d!",
                    net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, res);
                net_trans_task_set_done(task, net_trans_result_error, res);
                break;
            }
            break;
        }
        default:
            CPE_INFO(
                mgr->m_em, "%s: task %d (%s): check_multi_info recv msg %d!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, msg->msg);
            break;
        }
    }
}

static void net_trans_do_timer(EV_P_ struct ev_timer *w, int revents) {
    net_trans_manage_t mgr = w->data;
    CURLMcode rc;

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: curl_multi_socket_action(timeout)", net_trans_manage_name(mgr));
    }

    rc = curl_multi_socket_action(mgr->m_multi_handle, CURL_SOCKET_TIMEOUT, 0, &mgr->m_still_running);
    if (rc != CURLM_OK) {
        CPE_ERROR(mgr->m_em, "%s: do_timer: curl_multi_socket_action return error %d!", net_trans_manage_name(mgr), rc);
    }

    net_trans_check_multi_info(mgr);
}

static int net_trans_timer_cb(CURLM *multi, long timeout_ms, net_trans_manage_t mgr) {
    if (mgr->m_debug >= 2) {
        CPE_INFO(mgr->m_em, "%s: timer_cb: timeout_ms=%d", net_trans_manage_name(mgr), (int)timeout_ms);
    }

    ev_timer_stop(mgr->m_loop, &mgr->m_timer_event);

    if (timeout_ms > 0) {
        double  t = (double)timeout_ms / 1000.0;
        ev_timer_init(&mgr->m_timer_event, net_trans_do_timer, t, 0.);
        ev_timer_start(mgr->m_loop, &mgr->m_timer_event);
    }
    else {
        net_trans_do_timer(mgr->m_loop, &mgr->m_timer_event, 0);
    }

    return 0;
}
