#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "set_svr_set_conn_fsm.h"
#include "protocol/svr/set/set_share_pkg.h"

int set_svr_set_conn_send(set_svr_set_conn_t conn, uint16_t to_svr_id, dp_req_t body, dp_req_t head, dp_req_t carry, size_t * write_size) {
    set_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = 2048;
    ringbuffer_block_t blk;
    char * buf;
    int32_t encode_size;
    uint32_t pkg_size;
    SET_PKG_HEAD * input_head_buf;

RESIZE_AND_TRY_AGAIN:
    blk = set_svr_ringbuffer_alloc(svr, curent_pkg_size, conn->m_conn_id);
    if (blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: not enouth ringbuf, curent_pkg_size=%d, data-len=%d!",
            set_svr_name(svr),
            set_svr_set_name(conn->m_set), (int)curent_pkg_size, (int)pkg_size);
        return -1;
    }

    buf = NULL;
    ringbuffer_data(svr->m_ringbuf, blk, curent_pkg_size, 0, (void*)&buf);
    assert(buf);

    pkg_size = sizeof(uint32_t);

    /*写入包头*/
    input_head_buf = dp_req_data(head);
    

    CPE_COPY_HTON16(buf + pkg_size, &input_head_buf->to_svr_type);
    pkg_size += 2;
    CPE_COPY_HTON16(buf + pkg_size, &to_svr_id);
    pkg_size += 2;
    CPE_COPY_HTON16(buf + pkg_size, &input_head_buf->from_svr_type);
    pkg_size += 2;
    CPE_COPY_HTON16(buf + pkg_size, &input_head_buf->from_svr_id);
    pkg_size += 2;
    CPE_COPY_HTON32(buf + pkg_size, &input_head_buf->sn);
    pkg_size += 4;
    CPE_COPY_HTON16(buf + pkg_size, &input_head_buf->flags);
    pkg_size += 2;

    /*写入carry_info*/
    memcpy(buf + pkg_size, dp_req_data(carry), dp_req_size(carry));
    pkg_size += dp_req_size(carry);

    /*写入包体*/
    encode_size =
        dr_pbuf_write(
            buf + pkg_size, curent_pkg_size - pkg_size,
            dp_req_data(body), dp_req_size(body), dp_req_meta(body), svr->m_em);
    if (encode_size < 0) {
        if (encode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size >= svr->m_set_max_pkg_size) {
                CPE_ERROR(
                    svr->m_em, "%s: set %s: send: not enough encode buf, buf size is %d!",
                    set_svr_name(svr), set_svr_set_name(conn->m_set),
                    (int)curent_pkg_size);
                
                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                return -1;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > svr->m_set_max_pkg_size) curent_pkg_size = svr->m_set_max_pkg_size;

                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: set %s: send: encode fail, buf size is %d!",
                set_svr_name(svr), set_svr_set_name(conn->m_set),
                (int)curent_pkg_size);

            blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
            assert(blk == NULL);

            return -1;
        }
    }

    pkg_size += encode_size;

    CPE_COPY_HTON32(buf, &pkg_size);

    ringbuffer_shrink(conn->m_svr->m_ringbuf, blk, pkg_size);
    set_svr_set_conn_link_node_w_tail(conn, blk);

    if (write_size) *write_size = pkg_size;

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: set %s: ==> send one request, (net-size=%d)\n\thead: %s\tcarry: %s\tbody: %s",
            set_svr_name(svr), set_svr_set_name(conn->m_set),
            (int)pkg_size,
            dp_req_dump(head, &svr->m_dump_buffer_head),
            dp_req_dump(carry, &svr->m_dump_buffer_carry),
            dp_req_dump(body, &svr->m_dump_buffer_body));
    }

    set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_wb_update);

    return 0;
}
