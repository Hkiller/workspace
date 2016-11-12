#ifndef SVR_ACCOUNT_SVR_CONN_INFO_H
#define SVR_ACCOUNT_SVR_CONN_INFO_H
#include "account_svr_module.h"

struct account_svr_account_info {
    account_svr_t m_svr;
    struct cpe_hash_entry m_hh;
    SVR_ACCOUNT_LOGIC_ID const * m_id;
    uint8_t m_state;
};

/*account info*/
account_svr_account_info_t account_svr_account_info_create(account_svr_t svr, SVR_ACCOUNT_LOGIC_ID const * logic_id, uint8_t state);
void account_svr_account_info_free(account_svr_account_info_t account_info);
account_svr_account_info_t account_svr_account_info_find(account_svr_t svr, SVR_ACCOUNT_LOGIC_ID const * logic_id);

void account_svr_account_info_free_all(account_svr_t svr);
uint32_t account_svr_account_info_hash(account_svr_account_info_t account_info);
int account_svr_account_info_eq(account_svr_account_info_t l, account_svr_account_info_t r);

#endif
