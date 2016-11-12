#ifndef SVR_DIR_SVR_SERVER_H
#define SVR_DIR_SVR_SERVER_H
#include "dir_svr.h"

struct dir_svr_server {
    dir_svr_region_t m_region;
    char m_ip[16];
    uint16_t m_port;

    TAILQ_ENTRY(dir_svr_server) m_next;
};

/*server ops*/
dir_svr_server_t dir_svr_server_create(dir_svr_region_t region, const char * ip, uint16_t port);
void dir_svr_server_free(dir_svr_server_t server);

#endif
