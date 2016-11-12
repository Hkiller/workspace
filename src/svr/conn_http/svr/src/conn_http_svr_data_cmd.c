#include "cpe/dr/dr_metalib_manage.h"
#include "conn_http_svr_ops.h" 

conn_http_cmd_t
conn_http_cmd_create(conn_http_service_t service, const char * path, uint32_t id, LPDRMETA req_meta) {
    conn_http_svr_t svr = service->m_svr;
    conn_http_cmd_t cmd;
    size_t path_len = strlen(path) + 1;

    cmd = mem_alloc(svr->m_alloc, sizeof(struct conn_http_cmd) + path_len);
    if (cmd == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: service %s: cmd %s: create: alloc fail",
            conn_http_svr_name(svr), service->m_path, path);
        return NULL;
    }

    cmd->m_service = service;
    cmd->m_req_id = id;
    cmd->m_req_meta = req_meta;
    cmd->m_path = (char*)(cmd + 1);
    memcpy((char*)(cmd + 1), path, path_len);

    cmd->m_pkg_buf_size = req_meta ? dr_meta_size(req_meta) : 0;

    TAILQ_INSERT_TAIL(&service->m_cmds, cmd, m_next_for_service);

    return cmd;
}

void conn_http_cmd_free(conn_http_cmd_t cmd) {
    conn_http_service_t service = cmd->m_service;
    conn_http_svr_t svr = service->m_svr;

    TAILQ_REMOVE(&service->m_cmds, cmd, m_next_for_service);

    mem_free(svr->m_alloc, cmd);
}

void conn_http_cmd_free_all(conn_http_service_t service) {
    while(!TAILQ_EMPTY(&service->m_cmds)) {
        conn_http_cmd_free(TAILQ_FIRST(&service->m_cmds));
    }
}

conn_http_cmd_t
conn_http_cmd_find(conn_http_service_t service, const char * path) {
    conn_http_cmd_t cmd;

    TAILQ_FOREACH(cmd, &service->m_cmds, m_next_for_service) {
        if (strcmp(cmd->m_path, path) == 0) {
            return cmd;
        }
    }

    return NULL;
}
