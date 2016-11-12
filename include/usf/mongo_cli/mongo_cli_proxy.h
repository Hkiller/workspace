#ifndef USF_MONGO_CLI_PROXY_H
#define USF_MONGO_CLI_PROXY_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h" 
#include "gd/app/app_types.h"
#include "mongo_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_cli_proxy_t
mongo_cli_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    mongo_driver_t mongo_driver,
    mem_allocrator_t alloc,
    error_monitor_t em);

void mongo_cli_proxy_free(mongo_cli_proxy_t agent);

mongo_cli_proxy_t
mongo_cli_proxy_find(gd_app_context_t app, cpe_hash_string_t name);

mongo_cli_proxy_t
mongo_cli_proxy_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t mongo_cli_proxy_app(mongo_cli_proxy_t agent);
const char * mongo_cli_proxy_name(mongo_cli_proxy_t agent);
cpe_hash_string_t mongo_cli_proxy_name_hs(mongo_cli_proxy_t agent);

logic_manage_t mongo_cli_proxy_logic_manage(mongo_cli_proxy_t agent);
mongo_driver_t mongo_cli_proxy_driver(mongo_cli_proxy_t agent);

int mongo_cli_proxy_set_outgoing_send_to(mongo_cli_proxy_t agent, const char * outgoing_send_to);
int mongo_cli_proxy_set_incoming_recv_at(mongo_cli_proxy_t agent, const char * outgoing_send_to);

const char * mongo_cli_proxy_dft_db(mongo_cli_proxy_t agent);
void mongo_cli_proxy_set_dft_db(mongo_cli_proxy_t agent, const char * db_name);

mongo_pkg_t mongo_cli_proxy_pkg_buf(mongo_cli_proxy_t proxy);

int mongo_cli_proxy_send(
    mongo_cli_proxy_t agent, mongo_pkg_t pkg, logic_require_t require,
    LPDRMETA result_meta, int result_count_init, const char * result_prefix,
    mongo_cli_pkg_parser parser, void * parse_ctx);

#ifdef __cplusplus
}
#endif

#endif
