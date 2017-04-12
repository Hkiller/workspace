#ifndef USF_MONGO_CLI_INTERNAL_OPS_H
#define USF_MONGO_CLI_INTERNAL_OPS_H
#include "mongo_cli_internal_types.h"

int mongo_cli_proxy_recv(dp_req_t req, void * ctx, error_monitor_t em);
mongo_pkg_t mongo_cli_proxy_cmd_buf(mongo_cli_proxy_t proxy);

#endif
