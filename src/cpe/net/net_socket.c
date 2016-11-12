#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_unistd.h"
#include "net_internal_ops.h"

int net_socket_set_none_block(int fd, error_monitor_t em) {
    if (cpe_sock_set_none_block(fd, 1) != 0) {
        CPE_ERROR(em,
            "net_tcp_set_none_block: set non block fail! errno=%d (%s)",
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }
    else {
        return 0;
    }
}

int net_socket_set_reuseaddr(int fd, error_monitor_t em) {
    if (cpe_sock_set_reuseaddr(fd, 1) != 0) {
        CPE_ERROR(em,
            "net_tcp_set_reuseaddr: set reuseaddr fail! errno=%d (%s)",
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }
    else {
        return 0;
    }
}

void net_socket_close(int * fd, error_monitor_t em) {
    if (*fd == -1) return;

    if (cpe_sock_close(*fd) != 0) {
        CPE_ERROR(em, "net_socket_destory: close fail, errno=%d (%s)", cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
    }

    *fd = -1;
}
