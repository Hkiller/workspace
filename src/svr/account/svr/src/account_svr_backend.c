#include "cpe/cfg/cfg_read.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_stack.h"
#include "usf/logic/logic_require.h"
#include "account_svr_backend.h"
#include "account_svr_backend_qihoo.h"
#include "account_svr_backend_weixin.h"
#include "account_svr_backend_damai.h"

account_svr_backend_t
account_svr_backend_create(
    account_svr_t svr, uint8_t account_type,
    size_t capacity, cfg_t cfg,
    account_svr_backend_init_fun_t init,
    account_svr_backend_fini_fun_t fini,
    account_svr_backend_token_to_id_fun_t token_to_id)
{
    account_svr_backend_t backend;

    backend = mem_alloc(svr->m_alloc, sizeof(struct account_svr_backend) + capacity);
    if (backend == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_create: alloc fail!");
        return NULL;
    }

    backend->m_svr = svr;
    backend->m_account_type = account_type;
    backend->m_init = init;
    backend->m_fini = fini;
    backend->m_token_to_id = token_to_id;

    if (backend->m_init) {
        backend->m_init(backend, cfg);
    }

    TAILQ_INSERT_TAIL(&svr->m_backends, backend, m_next);

    return backend;
}

void account_svr_backend_free(account_svr_backend_t backend) {
    account_svr_t svr = backend->m_svr;

    if (backend->m_fini) backend->m_fini(backend);

    TAILQ_REMOVE(&svr->m_backends, backend, m_next);

    mem_free(svr->m_alloc, backend);
}

account_svr_backend_t account_svr_backend_find(account_svr_t svr, uint8_t account_type) {
    account_svr_backend_t backend;

    TAILQ_FOREACH(backend, &svr->m_backends, m_next) {
        if (backend->m_account_type == account_type) return backend;
    }

    return NULL;
}

void * account_svr_backend_data(account_svr_backend_t backend) {
    return backend + 1;
}

struct account_svr_backend_def {
    uint8_t account_type;
    size_t capacity;
    account_svr_backend_init_fun_t init;
    account_svr_backend_fini_fun_t fini;
    account_svr_backend_token_to_id_fun_t token_to_id;
};

static struct account_svr_backend_def s_backend_defs[] = {
    { SVR_ACCOUNT_DEVICE, 0, NULL, NULL, NULL }
    , { SVR_ACCOUNT_QIHOO,
        sizeof(struct account_svr_backend_qihoo),
        account_svr_backend_qihoo_init,
        account_svr_backend_qihoo_fini,
        account_svr_backend_qihoo_token_to_id }
    , { SVR_ACCOUNT_WEIXIN,
        sizeof(struct account_svr_backend_weixin),
        account_svr_backend_weixin_init,
        account_svr_backend_weixin_fini,
        account_svr_backend_weixin_token_to_id }
    , { SVR_ACCOUNT_DAMAI,
        sizeof(struct account_svr_backend_damai),
        account_svr_backend_damai_init,
        account_svr_backend_damai_fini,
        account_svr_backend_damai_token_to_id }
};

int account_svr_app_load_backends(account_svr_t svr, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg);
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * str_account_type = cfg_name(child_cfg);
        uint8_t account_type;
        struct account_svr_backend_def * backend_def = NULL;
        uint8_t i;

        account_type = account_svr_account_type_from_str(str_account_type);
        if (account_type == 0) {
            CPE_ERROR(svr->m_em, "account_svr_app_load_backends: account type %s unknown!", str_account_type);
            return -1;
        }

        for(i = 0; i < CPE_ARRAY_SIZE(s_backend_defs); ++i) {
            if (s_backend_defs[i].account_type == account_type) {
                backend_def = &s_backend_defs[i];
                break;
            }
        }

        if (backend_def) {
            account_svr_backend_t backend = account_svr_backend_create(
                svr,
                backend_def->account_type,
                backend_def->capacity,
                child_cfg,
                backend_def->init, backend_def->fini, backend_def->token_to_id);
            if (backend == NULL) {
                CPE_ERROR(svr->m_em, "account_svr_app_load_backends: backend %s create fail!", str_account_type);
                return -1;
            }

            CPE_INFO(svr->m_em, "account_svr_app_load_backends: backend %s(%d) create success!", str_account_type, account_type);
        }
        else {
            CPE_ERROR(svr->m_em, "account_svr_app_load_backends: backend %s(%d) create fail!", str_account_type, account_type);
            return -1;
        }
    }

    return 0;
}

logic_op_exec_result_t
account_svr_backend_check_send_logic_to_id_req(
    account_svr_backend_t backend, logic_stack_node_t stack, SVR_ACCOUNT_LOGIC_ID const * logic_id)
{
    account_svr_t svr = backend->m_svr;
    logic_require_t require;
    int rv;

    require = logic_require_create(stack, "token_to_id");
    if (require == NULL) {
        logic_context_errno_set(logic_stack_node_context(stack), -1);
        CPE_ERROR(svr->m_em, "account_svr_backend_check_send_logic_to_id_req: create require fail!");
        return logic_op_exec_result_false;
    }

    rv = backend->m_token_to_id(backend, logic_id, require);
    if (rv != 0) {
        logic_require_set_error_ex(require, rv);
        return logic_op_exec_result_false;
    }
    else {
        return logic_op_exec_result_true;
    }
}

