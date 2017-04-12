#ifndef ACCOUNT_SVR_TOKEN_INFO_H
#define ACCOUNT_SVR_TOKEN_INFO_H
#include "account_svr_module.h"

struct account_svr_login_info {
    account_svr_t m_svr;
    union {
        struct cpe_hash_entry m_hh;
        TAILQ_ENTRY(account_svr_login_info) m_next;
    };
    SVR_ACCOUNT_LOGIN_INFO m_data;
};

account_svr_login_info_t account_svr_login_info_create(account_svr_t svr, uint64_t account_id);
void account_svr_login_info_free(account_svr_login_info_t account_info);
void account_svr_login_info_free_all(account_svr_t svr);
void account_svr_login_info_real_free(account_svr_login_info_t account_info);

account_svr_login_info_t account_svr_login_info_find(account_svr_t svr, uint64_t account_id);
account_svr_login_info_t account_svr_login_check_create(account_svr_t svr, uint64_t account_id);

int account_svr_login_info_update(account_svr_t svr, uint64_t account_id, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_context_t ctx);

const char * account_svr_login_info_dump(account_svr_t svr, SVR_ACCOUNT_LOGIN_INFO const * login_info);

uint32_t account_svr_login_info_hash(account_svr_login_info_t login_info);
int account_svr_login_info_eq(account_svr_login_info_t l, account_svr_login_info_t r);

#endif
