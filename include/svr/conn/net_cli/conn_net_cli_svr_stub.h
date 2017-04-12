#ifndef SVR_CONN_CLI_SVR_STUB_H
#define SVR_CONN_CLI_SVR_STUB_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash_string.h"
#include "conn_net_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

conn_net_cli_svr_stub_t conn_net_cli_svr_stub_find_by_id(conn_net_cli_t cli, uint16_t svr_type);
conn_net_cli_svr_stub_t conn_net_cli_svr_stub_lsearch_by_name(conn_net_cli_t cli, const char * name);

const char * conn_net_cli_svr_stub_type_name(conn_net_cli_svr_stub_t svr);

cpe_hash_string_t conn_net_cli_svr_stub_response_dispatch_to(conn_net_cli_svr_stub_t svr);
int conn_net_cli_svr_stub_set_response_dispatch_to(struct conn_net_cli_svr_stub * svr, const char * response_dispatch_to);

cpe_hash_string_t conn_net_cli_svr_stub_notify_dispatch_to(conn_net_cli_svr_stub_t svr);
int conn_net_cli_svr_stub_set_notify_dispatch_to(struct conn_net_cli_svr_stub * svr, const char * notify_dispatch_to);

int conn_net_cli_svr_stub_set_outgoing_recv_at(conn_net_cli_svr_stub_t svr, const char * outgoing_recv_at);

LPDRMETA conn_net_cli_svr_stub_find_data_meta_by_cmd(conn_net_cli_svr_stub_t svr_info, uint32_t cmd);

LPDRMETA conn_net_cli_svr_stub_error_pkg_meta(conn_net_cli_svr_stub_t svr_info);
uint32_t conn_net_cli_svr_stub_error_pkg_cmd(conn_net_cli_svr_stub_t svr_info);
LPDRMETAENTRY conn_net_cli_svr_stub_error_pkg_errno_entry(conn_net_cli_svr_stub_t svr_info);

#ifdef __cplusplus
}
#endif

#endif
