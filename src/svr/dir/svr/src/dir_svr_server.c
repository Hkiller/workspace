#include "cpe/utils/string_utils.h"
#include "dir_svr_server.h"
#include "dir_svr_region.h"

dir_svr_server_t dir_svr_server_create(dir_svr_region_t region, const char * ip, uint16_t port) {
    dir_svr_t svr = region->m_svr;
    dir_svr_server_t server;

    server = mem_alloc(svr->m_alloc, sizeof(struct dir_svr_server));
    if (server == NULL) return NULL;

    server->m_region = region;
    cpe_str_dup(server->m_ip, sizeof(server->m_ip), ip);
    server->m_port = port;

    TAILQ_INSERT_TAIL(&region->m_servers, server, m_next);

    return server;
}

void dir_svr_server_free(dir_svr_server_t server) {
    dir_svr_region_t region = server->m_region;
    dir_svr_t svr = region->m_svr;

    TAILQ_REMOVE(&region->m_servers, server, m_next);

    mem_free(svr->m_alloc, server);
}
