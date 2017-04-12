#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "set_svr_center_fsm.h"

int set_svr_center_send(set_svr_center_t center, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    set_svr_t svr = center->m_svr;
    size_t curent_pkg_size = 2048;
    ringbuffer_block_t blk;
    char * buf;
    int32_t encode_size;

    while(curent_pkg_size < pkg_size) { 
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    blk = set_svr_ringbuffer_alloc(svr, curent_pkg_size, center->m_conn_id);
    if (blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: center: send: not enouth ringbuf, curent_pkg_size=%d, data-len=%d!",
            set_svr_name(svr), (int)curent_pkg_size, (int)pkg_size);
    }

    buf = NULL;
    ringbuffer_data(svr->m_ringbuf, blk, curent_pkg_size, 0, (void*)&buf);
    assert(buf);

    encode_size =
        dr_pbuf_write(
            buf + sizeof(uint32_t),
            curent_pkg_size - sizeof(uint32_t),
            pkg, pkg_size, center->m_pkg_meta, svr->m_em);
    if (encode_size < 0) {
        if (encode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size >= center->m_max_pkg_size) {
                CPE_ERROR(
                    center->m_svr->m_em, "%s: center: send: not enough encode buf, buf size is %d!",
                    set_svr_name(center->m_svr), (int)curent_pkg_size);

                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                return -1;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > center->m_max_pkg_size) curent_pkg_size = center->m_max_pkg_size;

                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else {
            CPE_ERROR(
                center->m_svr->m_em, "%s: center: send: encode fail, buf size is %d!",
                set_svr_name(center->m_svr),
                (int)curent_pkg_size);

            blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
            assert(blk == NULL);

            return -1;
        }
    }
    else {
        encode_size += sizeof(uint32_t);
        CPE_COPY_HTON32(buf, &encode_size);
    }

    ringbuffer_shrink(center->m_svr->m_ringbuf, blk, encode_size);
    set_svr_center_link_node_w(center, blk);

    if (center->m_svr->m_debug) {
        CPE_INFO(
            center->m_svr->m_em, "%s: center: ==> send one request, (net-size=%d, data-size=%d)\n%s",
            set_svr_name(center->m_svr),
            (int)encode_size, (int)pkg_size,
            dr_json_dump_inline(&center->m_svr->m_dump_buffer_body, pkg, pkg_size, center->m_pkg_meta));
    }

    set_svr_center_apply_evt(center, set_svr_center_fsm_evt_wb_update);

    return 0;
}
