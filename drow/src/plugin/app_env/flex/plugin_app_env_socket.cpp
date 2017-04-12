#include "cpe/pal/pal_stdio.h"
#include "plugin_app_env_socket_i.hpp"

extern plugin_app_env_backend_t g_app_env_flex;

extern "C"
int socket(int domain, int type, int protocol) {
    if (g_app_env_flex == NULL) return -1;
    CPE_ERROR(g_app_env_flex->m_module->m_em, "xxxxx: socket\n");
    return -1;
}

extern "C"
int pool(int domain, int type, int protocol) {
    printf("xxxxx pool 1\n");
    if (g_app_env_flex == NULL) return -1;
    printf("xxxxx pool 2\n");
    return 0;
}

extern "C"
int connect(int fd, const struct sockaddr * name, socklen_t namelen) {
    return -1;
}

extern "C"
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    return -1;
}

extern "C"
ssize_t sendto(int, const void*, size_t, int, const sockaddr*, socklen_t) {
    return -1;
}
