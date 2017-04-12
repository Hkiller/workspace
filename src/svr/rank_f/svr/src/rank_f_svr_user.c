#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_f_svr_ops.h"

int rank_f_svr_user_clear_index(rank_f_svr_t svr, uint64_t user_id) {
    uint16_t i;

    for(i = 0; i < (sizeof(svr->m_index_infos) / sizeof(svr->m_index_infos[0])); ++i) {
        rank_f_svr_index_t index;

        if (svr->m_index_infos[i].m_svr == NULL) continue;

        index = rank_f_svr_index_find(svr, user_id, i);
        if (index) {
            rank_f_svr_index_clear_records(svr, index);
            rank_f_svr_index_free(svr, index);
        }
    }

    return 0;
}

int rank_f_svr_user_destory(rank_f_svr_t svr, uint64_t user_id) {
    rank_f_svr_index_t gid_index;

    gid_index = rank_f_svr_index_find(svr, user_id, 0);
    if (gid_index == NULL) {
        CPE_INFO(
            svr->m_em, "%s: user "FMT_UINT64_T" destory: gid index not exist, skip destory!",
            rank_f_svr_name(svr), user_id);
        return 0;
    }

    if (rank_f_svr_user_clear_index(svr, user_id) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: user "FMT_UINT64_T" destory: clear index fail!",
            rank_f_svr_name(svr), user_id);
        return -1;
    }

    rank_f_svr_index_destory_records(svr, gid_index);
    rank_f_svr_index_free(svr, gid_index);

    return 0;
}

rank_f_svr_index_t rank_f_svr_user_check_create(rank_f_svr_t svr, uint64_t user_id) {
    rank_f_svr_index_t gid_index;

    gid_index = rank_f_svr_index_find(svr, user_id, 0);
    if (gid_index == NULL) {
        gid_index = rank_f_svr_index_alloc(svr, user_id, 0);
        if (gid_index == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: user "FMT_UINT64_T" create: alloc gid index fail!",
                rank_f_svr_name(svr), user_id);
            return NULL;
        }
    }

    assert(gid_index);
    return gid_index;
}

int rank_f_svr_user_index_check_create(rank_f_svr_index_t * r, rank_f_svr_t svr, uint64_t user_id, uint8_t index_id) {
    rank_f_svr_index_t idx;
    rank_f_svr_index_t gid_index;
    int rv;

    idx = rank_f_svr_index_find(svr, user_id, index_id);
    if (idx) {
        *r = idx;
        return 0;
    }

    gid_index = rank_f_svr_index_find(svr, user_id, 0);
    if (gid_index == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: user "FMT_UINT64_T" build index %d: user not exist!",
            rank_f_svr_name(svr), user_id, index_id);
        return SVR_RANK_F_ERROR_USER_NOT_EXIST;
    }

    idx = rank_f_svr_index_alloc(svr, user_id, index_id);
    if (idx == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: user "FMT_UINT64_T" build index %d: alloc index fail!",
            rank_f_svr_name(svr), user_id, index_id);
        return SVR_RANK_F_ERROR_INTERNAL;
    }

    rv = rank_f_svr_record_sort(svr, idx, gid_index);
    if (rv != 0) {
        CPE_ERROR(
            svr->m_em, "%s: user "FMT_UINT64_T" build index %d: sort fail!",
            rank_f_svr_name(svr), user_id, index_id);
        rank_f_svr_index_clear_records(svr, idx);
        rank_f_svr_index_free(svr, idx);
        return rv;
    }

    *r = idx;
    return 0;
}
