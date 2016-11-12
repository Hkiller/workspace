#ifndef SVR_CONN_CLI_TYPES_H
#define SVR_CONN_CLI_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct conn_net_cli * conn_net_cli_t;
typedef struct conn_net_cli_svr_stub  * conn_net_cli_svr_stub_t;
typedef struct conn_net_cli_pkg * conn_net_cli_pkg_t;

typedef enum conn_net_cli_state {
    conn_net_cli_state_disable
    , conn_net_cli_state_disconnected
    , conn_net_cli_state_connecting
    , conn_net_cli_state_established
} conn_net_cli_state_t;

typedef void (*conn_net_cli_state_process_fun_t)(conn_net_cli_t cli, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
