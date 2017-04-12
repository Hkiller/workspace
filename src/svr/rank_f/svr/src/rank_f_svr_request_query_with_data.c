#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

void rank_f_svr_request_query_with_data(rank_f_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    SVR_RANK_F_REQ_QUERY_WITH_DATA * req;
    rank_f_svr_index_t index;
    dp_req_t response_body;
    SVR_RANK_F_RES_QUERY_WITH_DATA * res;
    rank_f_svr_index_buf_t buf;
    int rv;
    uint32_t data_buf_capacity;
    uint8_t * data_buf;

    req = &((SVR_RANK_F_PKG*)dp_req_data(pkg_body))->data.svr_rank_f_req_query_with_data;

    rv = rank_f_svr_user_index_check_create(&index, svr, req->user_id, req->index_id);
    if (rv != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request query-with-data: check create index "FMT_UINT64_T".%d fail",
            rank_f_svr_name(svr), req->user_id, req->index_id);
        rank_f_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    data_buf_capacity = svr->m_data_size * index->m_record_count;

    response_body = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_RANK_F_PKG) + data_buf_capacity);

    res = set_svr_stub_pkg_to_data(svr->m_stub, response_body, 0, svr->m_pkg_meta_res_query_with_data, NULL);
    if (res == NULL) {
        CPE_ERROR(svr->m_em, "%s: request query-with-data: make response fail!", rank_f_svr_name(svr));
        rank_f_svr_send_error_response(svr, pkg_head, rv);
        return;
    }

    res->user_id = req->user_id;
    res->index_id = req->index_id;
    res->start_pos = req->start_pos;
    res->total_count = index->m_record_count;
    res->return_count = 0;
    res->data_len = 0;
    data_buf = res->data;

    for(buf = index->m_bufs; buf; buf = buf->m_next) {
        uint8_t i = 0;

        if (req->start_pos) {
            if (req->start_pos >= RANK_F_SVR_INDEX_BUF_RECORD_COUNT) {
                req->start_pos -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT;
                continue;
            }
            else {
                i = req->start_pos;
                req->start_pos = 0;
            }
        }

        for(; i < buf->m_record_count
                && data_buf_capacity > 0
                && (req->require_count == 0 || res->return_count < req->require_count);
            ++i)
        {
            SVR_RANK_F_RECORD * record = buf->m_records[i];
            int rv;

            assert(record);

            if (svr->m_debug >= 2) {
                struct mem_buffer buffer;
                mem_buffer_init(&buffer, svr->m_alloc);
            
                CPE_INFO(
                    svr->m_em, "%s: request query-with-data: dump record: %s",
                    rank_f_svr_name(svr), dr_json_dump(&buffer, record + 1, svr->m_data_size, svr->m_data_meta));

                mem_buffer_clear(&buffer);
            }

            rv = dr_pbuf_write_with_size(
                data_buf, data_buf_capacity, 
                record + 1, svr->m_data_size, svr->m_data_meta, svr->m_em);
            if (rv < 0) {
                CPE_ERROR(svr->m_em, "%s: request query-with-data: encode data fail!", rank_f_svr_name(svr));
                rank_f_svr_send_error_response(svr, pkg_head, -1);
                return;
            }

            assert(rv > 0 && rv <= data_buf_capacity);

            data_buf_capacity -= rv;
            data_buf += rv;
            res->return_count ++;
            res->data_len += rv;
        }
    }

    if (set_svr_stub_reply_pkg(svr->m_stub, pkg_body, response_body) != 0) {
        CPE_ERROR(svr->m_em, "%s: request query-with-data: send response fail!", rank_f_svr_name(svr));
    }
}
