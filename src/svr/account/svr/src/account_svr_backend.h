#ifndef SVR_ACCOUNT_SVR_BACKEND_H
#define SVR_ACCOUNT_SVR_BACKEND_H
#include "account_svr_module.h"

typedef int (*account_svr_backend_init_fun_t)(account_svr_backend_t backend, cfg_t cfg);
typedef void (*account_svr_backend_fini_fun_t)(account_svr_backend_t backend);
typedef int (*account_svr_backend_token_to_id_fun_t)(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require);

struct account_svr_backend {
    account_svr_t m_svr;
    TAILQ_ENTRY(account_svr_backend) m_next;
    uint8_t m_account_type;
    account_svr_backend_init_fun_t m_init;
    account_svr_backend_fini_fun_t m_fini;
    account_svr_backend_token_to_id_fun_t m_token_to_id;
};

account_svr_backend_t account_svr_backend_create(
    account_svr_t svr, uint8_t account_type,
    size_t capacity, cfg_t cfg,
    account_svr_backend_init_fun_t init,
    account_svr_backend_fini_fun_t fini,
    account_svr_backend_token_to_id_fun_t token_to_id);

void account_svr_backend_free(account_svr_backend_t backend);

account_svr_backend_t account_svr_backend_find(account_svr_t svr, uint8_t account_type);
void * account_svr_backend_data(account_svr_backend_t backend);

int account_svr_app_load_backends(account_svr_t svr, cfg_t cfg);

logic_op_exec_result_t
account_svr_backend_check_send_logic_to_id_req(
    account_svr_backend_t backend, logic_stack_node_t stack, SVR_ACCOUNT_LOGIC_ID const * logic_id);

#endif
