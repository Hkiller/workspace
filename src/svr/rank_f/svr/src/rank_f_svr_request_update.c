#include <assert.h> 
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

void rank_f_svr_request_update(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_F_REQ_UPDATE * req;
    rank_f_svr_index_t gid_index;
    const char * input_data;
    int32_t input_len;

    req = &((SVR_RANK_F_PKG*)dp_req_data(pkg_body))->data.svr_rank_f_req_update;

    if (rank_f_svr_user_clear_index(svr, req->user_id) != 0) {
        CPE_ERROR(svr->m_em, "%s: request update: clear index fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, -1);
        return;
    }

    gid_index = rank_f_svr_user_check_create(svr, req->user_id);
    if (gid_index == NULL) {
        CPE_ERROR(svr->m_em, "%s: request update: check create user fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, -1);
        return;
    }

    input_data = (const char *)req->data;
    input_len = (int32_t)req->data_len;

    while(input_len > 0) {
        char buf[svr->m_record_size];
        SVR_RANK_F_RECORD * new_record = (SVR_RANK_F_RECORD*)buf;
        size_t input_use = 0;
        if (dr_pbuf_read_with_size(
                new_record + 1, svr->m_data_size,
                input_data, input_len, &input_use, svr->m_data_meta,
                svr->m_em)
            < 0)
        {
            CPE_ERROR(svr->m_em, "%s: request update: decode data fail!", rank_f_svr_name(svr));
            rank_f_svr_send_error_response(svr, pkg_head, -1);
            return;
        }

        assert(input_use > 0);
        assert(input_use <= input_len);

        input_data += input_use;
        input_len -= input_use;

        new_record->rank_f_uid = req->user_id;

        if (svr->m_debug >= 2) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, svr->m_alloc);
            
            CPE_INFO(
                svr->m_em, "%s: request update: dump record: %s",
                rank_f_svr_name(svr), dr_json_dump(&buffer, new_record, svr->m_record_size, svr->m_record_meta));

            mem_buffer_clear(&buffer);
        }

        if (rank_f_svr_record_update(svr, gid_index, new_record) != 0) {
            CPE_ERROR(svr->m_em, "%s: request update: update record to gid index fail!", rank_f_svr_name(svr));
            rank_f_svr_send_error_response(svr, pkg_head, -1);
            return;
        }
    }

    if (set_pkg_sn(pkg_head)) {
        if (set_svr_stub_send_response_cmd(
                svr->m_stub,
                set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
                SVR_RANK_F_CMD_RES_UPDATE, NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: request update: send response fail!", rank_f_svr_name(svr));
        }
    }
}
