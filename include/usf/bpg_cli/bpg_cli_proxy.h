#ifndef USF_BPG_CLI_PROXY_H
#define USF_BPG_CLI_PROXY_H
#include "cpe/cfg/cfg_types.h"
#include "bpg_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_cli_proxy_t
bpg_cli_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    bpg_pkg_manage_t pkg_manage,
    error_monitor_t em);

void bpg_cli_proxy_free(bpg_cli_proxy_t proxy);

bpg_cli_proxy_t
bpg_cli_proxy_find(gd_app_context_t app, cpe_hash_string_t name);
bpg_cli_proxy_t
bpg_cli_proxy_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t bpg_cli_proxy_app(bpg_cli_proxy_t proxy);
const char * bpg_cli_proxy_name(bpg_cli_proxy_t proxy);
cpe_hash_string_t bpg_cli_proxy_name_hs(bpg_cli_proxy_t proxy);

LPDRMETALIB bpg_cli_proxy_metalib(bpg_cli_proxy_t proxy);
LPDRMETA bpg_cli_proxy_meta(bpg_cli_proxy_t proxy, const char * name);

size_t bpg_cli_proxy_buf_capacity(bpg_cli_proxy_t proxy);
void bpg_cli_proxy_set_buf_capacity(bpg_cli_proxy_t proxy, size_t capacity);
int bpg_cli_proxy_outgoing_set_send_to(bpg_cli_proxy_t proxy, cfg_t cfg);
int bpg_cli_proxy_outgoing_set_recv_at(bpg_cli_proxy_t proxy, const char * name);
int bpg_cli_proxy_incoming_set_no_sn_send_to(bpg_cli_proxy_t proxy, cfg_t cfg);

uint64_t bpg_cli_proxy_client_id(bpg_cli_proxy_t proxy);
void bpg_cli_proxy_set_client_id(bpg_cli_proxy_t proxy, uint64_t client_id);

bpg_pkg_manage_t bpg_cli_proxy_pkg_manage(bpg_cli_proxy_t proxy);

void * bpg_cli_proxy_data_buf(bpg_cli_proxy_t proxy);
dp_req_t bpg_cli_proxy_pkg_buf(bpg_cli_proxy_t proxy);

int bpg_cli_proxy_send(
    bpg_cli_proxy_t proxy,
    logic_require_t require,
    dp_req_t pkg);

#ifdef __cplusplus
}
#endif

#endif

