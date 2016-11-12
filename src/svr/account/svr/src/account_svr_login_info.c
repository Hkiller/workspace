#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "usf/logic/logic_data.h"
#include "account_svr_login_info.h"

account_svr_login_info_t account_svr_login_info_create(account_svr_t svr, uint64_t account_id) {
    account_svr_login_info_t login_info;

    login_info = TAILQ_FIRST(&svr->m_free_login_infos);
    if (login_info) {
        TAILQ_REMOVE(&svr->m_free_login_infos, login_info, m_next);
    }
    else {
        login_info = mem_alloc(svr->m_alloc, sizeof(struct account_svr_login_info));
        if (login_info == NULL) {
            CPE_ERROR(svr->m_em, "account_svr_login_info_create: alloc fail!");
            return NULL;
        }
    }

    bzero(login_info, sizeof(*login_info));
    login_info->m_svr = svr;
    login_info->m_data.account_id = account_id;

    cpe_hash_entry_init(&login_info->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_login_infos, login_info) != 0) {
        CPE_ERROR(svr->m_em, "account_svr_account_info_create: " FMT_UINT64_T " duplicate!", account_id);
        TAILQ_INSERT_TAIL(&svr->m_free_login_infos, login_info, m_next);
        return NULL;
    }

    return login_info;
}

void account_svr_login_info_free(account_svr_login_info_t login_info){
    account_svr_t svr = login_info->m_svr;

    cpe_hash_table_remove_by_ins(&svr->m_login_infos, login_info);

    TAILQ_INSERT_TAIL(&svr->m_free_login_infos, login_info, m_next);
}

void account_svr_login_info_free_all(account_svr_t svr){
    struct cpe_hash_it login_info_it;
    account_svr_login_info_t login_info;

    cpe_hash_it_init(&login_info_it, &svr->m_login_infos);

    login_info = cpe_hash_it_next(&login_info_it);
    while (login_info) {
        account_svr_login_info_t next = cpe_hash_it_next(&login_info_it);
        account_svr_login_info_free(login_info);
        login_info = next;
    }
}

void account_svr_login_info_real_free(account_svr_login_info_t login_info) {
    account_svr_t svr = login_info->m_svr;
    mem_free(svr->m_alloc, login_info);
}

account_svr_login_info_t account_svr_login_info_find(account_svr_t svr, uint64_t account_id){
    struct account_svr_login_info key;
    key.m_data.account_id = account_id;
    return cpe_hash_table_find(&svr->m_login_infos, &key);
}

const char * account_svr_login_info_dump(account_svr_t svr, SVR_ACCOUNT_LOGIN_INFO const * login_info) {
    return dr_json_dump_inline(&svr->m_dump_buffer, login_info, sizeof(*login_info), svr->m_meta_login_info);
}

int account_svr_login_info_update(account_svr_t svr, uint64_t account_id, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_context_t ctx) {
    logic_data_t login_info_data;
    account_svr_login_info_t cached_login_info;

    cached_login_info = account_svr_login_info_find(svr, account_id);
    if (cached_login_info == NULL) {
        cached_login_info = account_svr_login_info_create(svr, account_id);
        if (cached_login_info == NULL) {
            CPE_ERROR(svr->m_em, "%s: login: update login info: create cache fail!", account_svr_name(svr));
            return -1;
        }
    }

    assert(cached_login_info);
    assert(cached_login_info->m_data.account_id == account_id);

    login_info_data = logic_context_data_find(ctx, dr_meta_name(svr->m_meta_login_info));
    if (login_info_data) {
        cached_login_info->m_data = * (SVR_ACCOUNT_LOGIN_INFO*) logic_data_data(login_info_data);
    }
    else {
        bzero(&cached_login_info->m_data, sizeof(cached_login_info->m_data));
        cached_login_info->m_data.logic_id = *logic_id;
    }

    cached_login_info->m_data.account_id = account_id;

    return 0;
}

uint32_t account_svr_login_info_hash(account_svr_login_info_t login_info){
    return login_info->m_data.account_id;
}

int account_svr_login_info_eq(account_svr_login_info_t l, account_svr_login_info_t r){
    return l->m_data.account_id == r->m_data.account_id;
}
