#ifndef SVR_ACCOUNT_SVR_BACKEND_WEIXIN_H
#define SVR_ACCOUNT_SVR_BACKEND_WEIXIN_H
#include "gd/net_trans/net_trans_types.h"
#include "account_svr_backend.h"

typedef struct account_svr_backend_weixin * account_svr_backend_weixin_t;

struct account_svr_backend_weixin {
    net_trans_group_t m_trans_group;
};

int account_svr_backend_weixin_init(account_svr_backend_t backend, cfg_t cfg);
void account_svr_backend_weixin_fini(account_svr_backend_t backend);
int account_svr_backend_weixin_token_to_id(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require);

#endif
