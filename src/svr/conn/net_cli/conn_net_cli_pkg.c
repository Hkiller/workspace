#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "svr/conn/net_cli/conn_net_cli_pkg.h"
#include "conn_net_cli_internal_ops.h"

conn_net_cli_pkg_t
conn_net_cli_pkg_create(conn_net_cli_t cli) {
    dp_req_t dp_req;
    conn_net_cli_pkg_t pkg;

    dp_req = dp_req_create(gd_app_dp_mgr(cli->m_app), sizeof(struct conn_net_cli_pkg));
    if (dp_req == NULL) return NULL;

    dp_req_set_type(dp_req, req_type_conn_net_cli_pkg);

    pkg = (conn_net_cli_pkg_t)dp_req_data(dp_req);

    pkg->m_cli = cli;
    pkg->m_dp_req = dp_req;

    conn_net_cli_pkg_init(pkg);

    return pkg;
}

void conn_net_cli_pkg_free(conn_net_cli_pkg_t pkg) {
    dp_req_free(pkg->m_dp_req);
}

conn_net_cli_t conn_net_cli_pkg_agent(conn_net_cli_pkg_t pkg) {
    return pkg->m_cli;
}

void conn_net_cli_pkg_init(conn_net_cli_pkg_t pkg) {
    pkg->m_svr_type = 0;
    pkg->m_result = 0;
    pkg->m_flags = 0;
    pkg->m_sn = 0;
}

uint16_t conn_net_cli_pkg_svr_type(conn_net_cli_pkg_t pkg) {
    return pkg->m_svr_type;
}

void conn_net_cli_pkg_set_svr_type(conn_net_cli_pkg_t pkg, uint16_t svr_type) {
    pkg->m_svr_type = svr_type;
}

uint8_t conn_net_cli_pkg_result(conn_net_cli_pkg_t pkg) {
    return pkg->m_result;
}

void conn_net_cli_pkg_set_result(conn_net_cli_pkg_t pkg, uint8_t result) {
    pkg->m_result = result;
}

uint8_t conn_net_cli_pkg_flags(conn_net_cli_pkg_t pkg) {
    return pkg->m_flags; 
}

void conn_net_cli_pkg_set_flags(conn_net_cli_pkg_t pkg, uint8_t flags) {
    pkg->m_flags = flags;
}

uint32_t conn_net_cli_pkg_sn(conn_net_cli_pkg_t pkg) {
    return pkg->m_sn;
}

void conn_net_cli_pkg_set_sn(conn_net_cli_pkg_t pkg, uint32_t sn) {
    pkg->m_sn = sn;
}

dp_req_t conn_net_cli_pkg_to_dp_req(conn_net_cli_pkg_t req) {
    return req->m_dp_req;
}

conn_net_cli_pkg_t conn_net_cli_pkg_find(dp_req_t req) {
    dp_req_t pkg = dp_req_child_find(req, req_type_conn_net_cli_pkg);
    return pkg ? (conn_net_cli_pkg_t)dp_req_data(pkg) : NULL;
}

const char * req_type_conn_net_cli_pkg = "conn_net_cli_pkg";
