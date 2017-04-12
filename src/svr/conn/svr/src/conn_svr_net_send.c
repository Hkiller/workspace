#include <limits.h>
#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_pbuf.h"
#include "svr/set/share/set_pkg.h"
#include "protocol/svr/conn/svr_conn_net.h"
#include "conn_svr_ops.h"

int conn_svr_conn_net_send(conn_svr_conn_t conn, uint16_t from_svr_type, int8_t err, uint32_t sn, void const * data, uint16_t data_len, LPDRMETA meta) {
    conn_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = 2048;
    ringbuffer_block_t blk;
    SVR_CONN_NET_RES_HEAD * head;
    void * buf;
    size_t body_len;
    uint32_t pkg_len;

    while(curent_pkg_size < data_len) {
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    if (conn_svr_conn_alloc(&blk, svr, conn, curent_pkg_size) != 0) return -1;
    assert(blk);

    ringbuffer_data(svr->m_ringbuf, blk, curent_pkg_size, 0, &buf);
    assert(buf);

    head = buf;
    head->result = err;
    head->flags = 0;
    CPE_COPY_HTON16(&head->from_svr, &from_svr_type);
    CPE_COPY_HTON32(&head->sn, &sn);

    if (data) {
        int encode_size;
        assert(meta);

        encode_size =
            dr_pbuf_write(
                head + 1, curent_pkg_size - sizeof(SVR_CONN_NET_RES_HEAD),
                data, data_len, meta, svr->m_em);
        if (encode_size < 0) {
            if (encode_size == dr_code_error_not_enough_output) {
                if (curent_pkg_size >= svr->m_max_pkg_size) {
                    CPE_ERROR(
                        svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): send: not enough encode buf, buf size is %d!",
                        conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, (int)curent_pkg_size);

                    blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                    assert(blk == NULL);

                    return -1;
                }
                else {
                    curent_pkg_size *= 2;
                    if (curent_pkg_size > svr->m_max_pkg_size) curent_pkg_size = svr->m_max_pkg_size;

                    blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                    assert(blk == NULL);

                    goto RESIZE_AND_TRY_AGAIN;
                }
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): send: encode fail, buf size is %d!",
                    conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, (int)curent_pkg_size);

                blk = ringbuffer_yield(svr->m_ringbuf, blk, curent_pkg_size);
                assert(blk == NULL);

                return -1;
            }
        }

        body_len = encode_size;
    }
    else {
        body_len = 0;
    }

    pkg_len = sizeof(SVR_CONN_NET_RES_HEAD) + body_len;

    CPE_COPY_HTON16(&head->pkg_len, &pkg_len);
    
    ringbuffer_shrink(svr->m_ringbuf, blk, pkg_len);
    conn_svr_conn_link_node_w(conn, blk);

    ev_io_stop(svr->m_ev_loop, &conn->m_watcher);
    conn_svr_conn_start_watch(conn);
    
    return 0;
}

