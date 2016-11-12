#include "account_svr_conn_info.h"

account_svr_account_info_t
account_svr_account_info_create(account_svr_t svr, SVR_ACCOUNT_LOGIC_ID const * logic_id, uint8_t state) {
    account_svr_account_info_t account_info;

    account_info = mem_alloc(svr->m_alloc, sizeof(struct account_svr_account_info) + sizeof(SVR_ACCOUNT_LOGIC_ID));
    if (account_info == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_account_info_create: alloc fail!");
        return NULL;
    }

    memcpy(account_info + 1, logic_id, sizeof(*logic_id));
    account_info->m_svr = svr;
    account_info->m_id = (void*)(account_info + 1);
    account_info->m_state = state;

    cpe_hash_entry_init(&account_info->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_account_infos, account_info) != 0) {
        CPE_ERROR(svr->m_em, "account_svr_account_info_create: %d:%s duplicate!", logic_id->account_type, logic_id->account);
        mem_free(svr->m_alloc, account_info);
        return NULL;
    }

    return account_info;
}

void account_svr_account_info_free(account_svr_account_info_t account_info) {
    account_svr_t svr = account_info->m_svr;

    cpe_hash_table_remove_by_ins(&svr->m_account_infos, account_info);

    mem_free(svr->m_alloc, account_info);
}

account_svr_account_info_t
account_svr_account_info_find(account_svr_t svr, SVR_ACCOUNT_LOGIC_ID const * logic_id) {
    struct account_svr_account_info key;
    key.m_id = logic_id;
    return cpe_hash_table_find(&svr->m_account_infos, &key);
}

void account_svr_account_info_free_all(account_svr_t svr) {
    struct cpe_hash_it account_info_it;
    account_svr_account_info_t account_info;

    cpe_hash_it_init(&account_info_it, &svr->m_account_infos);

    account_info = cpe_hash_it_next(&account_info_it);
    while (account_info) {
        account_svr_account_info_t next = cpe_hash_it_next(&account_info_it);
        account_svr_account_info_free(account_info);
        account_info = next;
    }
}

uint32_t account_svr_account_info_hash(account_svr_account_info_t account_info) {
    return cpe_hash_str(account_info->m_id->account, strlen(account_info->m_id->account));
}

int account_svr_account_info_eq(account_svr_account_info_t l, account_svr_account_info_t r) {
    if (l->m_id->account_type != r->m_id->account_type) return 0;
    return strcmp(l->m_id->account, r->m_id->account) == 0 ? 1 : 0;
}

