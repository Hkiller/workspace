#ifndef SVR_CONN_CLI_H
#define SVR_CONN_CLI_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "conn_net_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

conn_net_cli_t conn_net_cli_create(
    gd_app_context_t app,
    const char * name, 
    mem_allocrator_t alloc, error_monitor_t em);

void conn_net_cli_free(conn_net_cli_t mgr);

conn_net_cli_t conn_net_cli_find(gd_app_context_t app, cpe_hash_string_t name);
conn_net_cli_t conn_net_cli_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t conn_net_cli_app(conn_net_cli_t mgr);
const char * conn_net_cli_name(conn_net_cli_t mgr);
cpe_hash_string_t conn_net_cli_name_hs(conn_net_cli_t mgr);

int conn_net_cli_set_svr(conn_net_cli_t cli, const char * ip, uint16_t port);
const char * conn_net_cli_svr_ip(conn_net_cli_t cli);
uint16_t conn_net_cli_svr_port(conn_net_cli_t cli);

const char * conn_net_cli_state_name(conn_net_cli_state_t state);

conn_net_cli_state_t conn_net_cli_state(conn_net_cli_t cli);
void conn_net_cli_enable(conn_net_cli_t cli);
void conn_net_cli_disable(conn_net_cli_t cli);

int conn_net_cli_monitor_add(conn_net_cli_t cli, conn_net_cli_state_process_fun_t process_fun, void * process_ctx);
int conn_net_cli_monitor_remove(conn_net_cli_t cli, conn_net_cli_state_process_fun_t process_fun, void * process_ctx);

int conn_net_cli_send(
    conn_net_cli_t cli,
    uint16_t to_svr, uint32_t sn,
    LPDRMETA meta, void const * data, uint16_t data_len);

int conn_net_cli_send_data(
    conn_net_cli_t cli,
    uint16_t to_svr, uint32_t sn,
    LPDRMETA meta, void const * data, uint16_t data_size);

int conn_net_cli_send_cmd(
    conn_net_cli_t cli,
    uint16_t to_svr, uint32_t sn, uint32_t cmd);

int conn_net_cli_read_data(
    conn_net_cli_t cli, conn_net_cli_svr_stub_t svr_info, dp_req_t pkg,
    uint32_t * r_cmd, LPDRMETA * r_meta, void ** r_data, size_t * r_data_size);

#ifdef __cplusplus
}
#endif

#endif
