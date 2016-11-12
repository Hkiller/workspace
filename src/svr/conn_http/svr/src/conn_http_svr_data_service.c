#include "conn_http_svr_ops.h" 

conn_http_service_t
conn_http_service_create(conn_http_svr_t svr, const char * svr_path, set_svr_svr_info_t dispatch_to, conn_http_formator_t formator) {
    conn_http_service_t service;
    size_t svr_path_len = strlen(svr_path) + 1;

    service = mem_alloc(svr->m_alloc, sizeof(struct conn_http_service) + svr_path_len);
    if (service == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: service %d(%s): create: alloc fail",
            conn_http_svr_name(svr), svr->m_port, svr_path);
        return NULL;
    }

    service->m_svr = svr;
    service->m_dispatch_to = dispatch_to;
    service->m_formator = formator;

    service->m_path = (char*)(service + 1);
    memcpy(service + 1, svr_path, svr_path_len);

    TAILQ_INIT(&service->m_cmds);

    TAILQ_INSERT_TAIL(&svr->m_services, service, m_next_for_svr);

    return service;
}

void conn_http_service_free(conn_http_service_t service) {
    conn_http_svr_t svr = service->m_svr;

    conn_http_cmd_free_all(service);

    TAILQ_REMOVE(&svr->m_services, service, m_next_for_svr);

    mem_free(svr->m_alloc, service);
}

void conn_http_service_free_all(conn_http_svr_t svr) {
    while(!TAILQ_EMPTY(&svr->m_services)) {
        conn_http_service_free(TAILQ_FIRST(&svr->m_services));
    }
}

conn_http_service_t
conn_http_service_find(conn_http_svr_t svr, const char * path) {
    conn_http_service_t service;

    TAILQ_FOREACH(service, &svr->m_services, m_next_for_svr) {
        if (strcmp(service->m_path, path) == 0) {
            return service;
        }
    }

    return NULL;
}
