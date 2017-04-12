#ifndef SVR_ACCOUNT_SVR_BACKEND_FACEBOOK_H
#define SVR_ACCOUNT_SVR_BACKEND_FACEBOOK_H
#include "yajl/yajl_tree.h"
#include "gd/net_trans/net_trans_types.h"
#include "account_svr_backend.h"

typedef struct account_svr_backend_facebook * account_svr_backend_facebook_t;

struct account_svr_backend_facebook {
    net_trans_group_t m_trans_group;
    uint8_t m_exchange_token;
    char * m_app_id;
    char * m_app_secret;
    const char * m_url;
};

int account_svr_backend_facebook_init(account_svr_backend_t backend, cfg_t cfg);
void account_svr_backend_facebook_fini(account_svr_backend_t backend);
int account_svr_backend_facebook_token_to_id(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require);
int account_svr_backend_facebook_query_friends(account_svr_backend_t backend, account_svr_login_info_t login_info, logic_require_t require);

int account_svr_backend_facebook_check_error(account_svr_backend_t backend, yajl_val data_tree, const char * op);

#endif
