#ifndef SVR_CONN_CLI_PKG_H
#define SVR_CONN_CLI_PKG_H
#include "cpe/utils/buffer.h"
#include "conn_net_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_conn_net_cli_pkg;

conn_net_cli_pkg_t conn_net_cli_pkg_create(conn_net_cli_t cli);
void conn_net_cli_pkg_free(conn_net_cli_pkg_t pkg);
conn_net_cli_t conn_net_cli_pkg_cli(conn_net_cli_pkg_t pkg);

void conn_net_cli_pkg_init(conn_net_cli_pkg_t pkg);

uint16_t conn_net_cli_pkg_svr_type(conn_net_cli_pkg_t pkg);
void conn_net_cli_pkg_set_svr_type(conn_net_cli_pkg_t pkg, uint16_t svr_type);

uint8_t conn_net_cli_pkg_result(conn_net_cli_pkg_t pkg);
void conn_net_cli_pkg_set_result(conn_net_cli_pkg_t pkg, uint8_t result);

uint8_t conn_net_cli_pkg_flags(conn_net_cli_pkg_t pkg);
void conn_net_cli_pkg_set_flags(conn_net_cli_pkg_t pkg, uint8_t flags);

uint32_t conn_net_cli_pkg_sn(conn_net_cli_pkg_t pkg);
void conn_net_cli_pkg_set_sn(conn_net_cli_pkg_t pkg, uint32_t sn);

dp_req_t conn_net_cli_pkg_to_dp_req(conn_net_cli_pkg_t pkg);
conn_net_cli_pkg_t conn_net_cli_pkg_find(dp_req_t pkg);

int conn_net_cli_pkg_read_data(conn_net_cli_pkg_t pkg, uint32_t * r_cmd, LPDRMETA * r_meta, void ** r_data, size_t * r_data_size);

#ifdef __cplusplus
}
#endif

#endif
