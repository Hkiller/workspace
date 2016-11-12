#ifndef SVR_SET_SVR_LISTENER_H
#define SVR_SET_SVR_LISTENER_H
#include "ev.h"
#include "set_svr.h"

struct set_svr_listener {
    set_svr_t m_svr;
    char m_ip[16];
    uint16_t m_port;

    int m_fd;
    struct ev_io m_watcher;
};

set_svr_listener_t set_svr_listener_create(set_svr_t svr);
void set_svr_listener_free(set_svr_listener_t listener);

int set_svr_listener_start(set_svr_listener_t listener, const char * ip, uint16_t port, int accept_queue_size);
void set_svr_listener_stop(set_svr_listener_t listener);

#endif
