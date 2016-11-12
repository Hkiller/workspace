#ifndef SVR_ACCOUNT_SVR_BACKEND_DAMAI_H
#define SVR_ACCOUNT_SVR_BACKEND_DAMAI_H
#include "gd/net_trans/net_trans_types.h"
#include "account_svr_backend.h"

typedef struct account_svr_backend_damai * account_svr_backend_damai_t;

struct account_svr_backend_damai {
    net_trans_group_t m_trans_group;
    char * m_app_id;
};

int account_svr_backend_damai_init(account_svr_backend_t backend, cfg_t cfg);
void account_svr_backend_damai_fini(account_svr_backend_t backend);
int account_svr_backend_damai_token_to_id(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require);

#endif
