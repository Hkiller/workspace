#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

void rank_f_svr_request_remove(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_F_REQ_REMOVE * req;
    rank_f_svr_index_t gid_index;
    uint16_t gid_start_pos;
    uint32_t i;

    req = &((SVR_RANK_F_PKG*)dp_req_data(pkg_body))->data.svr_rank_f_req_remove;

    if (rank_f_svr_user_clear_index(svr, req->user_id) != 0) {
        CPE_ERROR(svr->m_em, "%s: request remove: clear index fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, -1);
        return;
    }

    gid_index = rank_f_svr_user_check_create(svr, req->user_id);
    if (gid_index == NULL) {
        CPE_ERROR(svr->m_em, "%s: request remove: check create user fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, -1);
        return;
    }

    gid_start_pos = rank_f_svr_gid_start_pos(svr);

    for(i = 0; i < req->user_id_count; ++i) {
        char buf[svr->m_record_size];
        SVR_RANK_F_RECORD * key = (SVR_RANK_F_RECORD*)buf;

        key->rank_f_uid = req->user_id;
        *(uint64_t *)(((char const *)(key + 1)) + gid_start_pos) = req->user_ids[i];

        if (rank_f_svr_record_remove(svr, gid_index, key) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: request remove: remove record of "FMT_UINT64_T" fail!",
                rank_f_svr_name(svr), req->user_ids[i]);
            rank_f_svr_send_error_response(svr, pkg_head, -1);
            return;
        }
    }

    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_send_response_cmd(
                svr->m_stub,
                set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
                SVR_RANK_F_CMD_RES_REMOVE, NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: request remove: send response fail!", rank_f_svr_name(svr));
        }
    }
}
