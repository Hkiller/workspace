#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/app/app_context.h"
#include "center_svr_conn.h"
#include "center_svr_set_proxy.h"

static center_svr_conn_op_t g_center_cli_ops[] = {
    NULL
    , center_svr_conn_op_join, NULL
    , center_svr_conn_op_query_by_type, NULL
};

void center_svr_listener_cb(EV_P_ ev_io *w, int revents) {
    center_svr_t svr;
    center_svr_conn_t conn;
    int new_fd;

    svr = w->data;
    assert(svr);

    new_fd = cpe_accept(svr->m_fd, 0, 0);
    if (new_fd == -1) {
        CPE_ERROR(
            svr->m_em, "%s: accept error, errno=%d (%s)",
            center_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return;
    }

    if (cpe_sock_set_none_block(new_fd, 1) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: set %d: conn %d: set none block fail! errno=%d (%s)",
            center_svr_name(svr), 0, new_fd, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(new_fd);
        return;
    }

    conn = center_svr_conn_create(svr, new_fd);
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: create center_svr_conn fail!", center_svr_name(svr));
        cpe_sock_close(new_fd);
        return;
    }

    if(svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: accept success!", center_svr_name(svr));
    }
}


static int center_svr_conn_process_data(center_svr_conn_t conn);

void center_svr_conn_rw_cb(EV_P_ ev_io *w, int revents) {
    center_svr_conn_t conn = w->data;
    center_svr_t svr = conn->m_svr;
    char * buffer;

    if (revents & EV_READ) {
        ringbuffer_block_t blk;

        blk = ringbuffer_alloc(svr->m_ringbuf , svr->m_read_block_size);
        if (blk == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: set %d: conn %d: recv not enouth ringbuf, len=%d!",
                center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd,
                (int)svr->m_read_block_size);
            center_svr_conn_free(conn);
            return;
        }

        buffer = NULL;
        ringbuffer_block_data(svr->m_ringbuf, blk, 0, (void **)&buffer);
        assert(buffer);

        for(;;) {
            int bytes = cpe_recv(conn->m_fd, buffer, svr->m_read_block_size, 0);
            if (bytes > 0) {
                if (svr->m_debug >= 2) {
                    CPE_INFO(
                        svr->m_em, "%s: set %d: conn %d: recv %d bytes data!",
                        center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd, bytes);
                }

                ringbuffer_shrink(svr->m_ringbuf, blk, bytes);
                center_svr_conn_link_node_r(conn, blk);
                break;
            }
            else if (bytes == 0) {
                blk = ringbuffer_yield(svr->m_ringbuf, blk, svr->m_read_block_size);
                assert(blk == NULL);
                CPE_ERROR(
                    svr->m_em, "%s: set %d: conn %d: free for recv return 0!",
                    center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd);
                center_svr_conn_free(conn);
                return;
            }
            else {
                assert(bytes == -1);

                switch(errno) {
                case EWOULDBLOCK:
                case EINPROGRESS:
                    blk = ringbuffer_yield(svr->m_ringbuf, blk, svr->m_read_block_size);
                    assert(blk == NULL);
                    break;
                case EINTR:
                    continue;
                default:
                    CPE_ERROR(
                        svr->m_em, "%s: set %d: conn %d: free for recv error, errno=%d (%s)!",
                        center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd,
                        cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                    blk = ringbuffer_yield(svr->m_ringbuf, blk, svr->m_read_block_size);
                    assert(blk == NULL);
                    center_svr_conn_free(conn);
                    return;
                }
            }
        }

        if (center_svr_conn_process_data(conn) != 0) return;
    }

    if (revents & EV_WRITE) {
        while(conn->m_wb) {
            void * data;
            int block_size;
            int bytes;

            block_size = ringbuffer_block_data(svr->m_ringbuf, conn->m_wb, 0, &data);
            assert(block_size > 0);
            assert(data);

            bytes = cpe_send(conn->m_fd, data, block_size, 0);
            if (bytes > 0) {
                if (svr->m_debug >= 2) {
                    CPE_INFO(
                        svr->m_em, "%s: set %d: conn %d: send %d bytes data!",
                        center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd, bytes);
                }

                conn->m_wb = ringbuffer_yield(svr->m_ringbuf, conn->m_wb, bytes);

                if (bytes < block_size) break;
            }
            else if (bytes == 0) {
                CPE_ERROR(
                    svr->m_em, "%s: set %d: conn %d: free for send return 0!",
                    center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd);
                center_svr_conn_free(conn);
                return;
            }
            else {
                int err = cpe_sock_errno();
                assert(bytes == -1);

                if (err == EWOULDBLOCK || err == EINPROGRESS) break;
                if (err == EINTR) continue;

                CPE_ERROR(
                    svr->m_em, "%s: set %d: conn %d: free for send error, errno=%d (%s)!",
                    center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd, err, cpe_sock_errstr(err));

                center_svr_conn_free(conn);

                return;
            }
        }
    }

    ev_io_stop(svr->m_ev_loop, &conn->m_watcher);
    center_svr_conn_start_watch(conn);
}

static void * center_svr_conn_merge_rb(center_svr_conn_t conn) {
    center_svr_t svr = conn->m_svr;
    int length = ringbuffer_block_total_len(svr->m_ringbuf, conn->m_rb);
    ringbuffer_block_t new_blk;
    void * buf;

    new_blk = ringbuffer_alloc(svr->m_ringbuf, length);
    if (new_blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn: recv: not enouth ringbuf, len=%d!",
            center_svr_name(svr), (int)length);
        center_svr_conn_free(conn);
        return NULL;
    }

    buf = ringbuffer_copy(svr->m_ringbuf, conn->m_rb, 0, new_blk);
    assert(buf);

    ringbuffer_free(svr->m_ringbuf, conn->m_rb);
    conn->m_rb = new_blk;

    return buf;
}

static void * center_svr_conn_decode_pkg_buf(center_svr_conn_t conn, size_t * decode_len, LPDRMETA meta, void const * data, size_t data_len) {
    center_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = 2048;
    void * buf;
    int decode_size;

    while(curent_pkg_size < data_len) { 
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    if (conn->m_tb) ringbuffer_free(svr->m_ringbuf, conn->m_tb);

    conn->m_tb = ringbuffer_alloc(svr->m_ringbuf, curent_pkg_size);
    if (conn->m_tb == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: decode: not enouth ringbuf, to decode pkg, data-len=%d, require-buf-len=%d!",
            center_svr_name(svr), (int)data_len, (int)curent_pkg_size);
        return NULL;
    }
    conn->m_tb->id = conn->m_fd;

    buf = NULL;
    ringbuffer_data(svr->m_ringbuf, conn->m_tb, curent_pkg_size, 0, &buf);
    assert(buf);
    bzero(buf, curent_pkg_size);

    decode_size = dr_pbuf_read(buf, curent_pkg_size, data, data_len, meta, svr->m_em);
    if (decode_size < 0) {
        if (decode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size < svr->m_max_pkg_size) {
                curent_pkg_size *= 2;

                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: decode: decode require buf not enouth, resize to %d, input_data_len=%d",
                        center_svr_name(svr), (int)curent_pkg_size, (int)data_len);
                }

                goto RESIZE_AND_TRY_AGAIN;
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: decode: decode require buf too big!, curent_pkg_size=%d, input_data_len=%d",
                    center_svr_name(svr), (int)curent_pkg_size, (int)data_len);
                return NULL;
            }
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: decode: decode fail!, curent_pkg_size=%d, input_data_len=%d",
                center_svr_name(svr), (int)curent_pkg_size, (int)data_len);
            return NULL;
        }
    }

    if (decode_len) *decode_len = decode_size;

    return buf;
}

static int center_svr_conn_process_data(center_svr_conn_t conn) {
    center_svr_t svr = conn->m_svr;
    void * buf;
    int received_data_len;
    uint32_t pkg_data_len;
    size_t pkg_decode_len;
    SVR_CENTER_PKG * pkg;

    while(conn->m_rb) {
        received_data_len = ringbuffer_data(svr->m_ringbuf, conn->m_rb, sizeof(pkg_data_len), 0, &buf);
        if (received_data_len < sizeof(pkg_data_len)) return 0; /*缓存数据不够读取包长度*/

         /*数据主够读取包的大小，但是头块太小，无法保存数据头，提前合并一次数据*/
        if (buf == NULL) buf = center_svr_conn_merge_rb(conn);
        if (buf == NULL) return -1;

        CPE_COPY_HTON32(&pkg_data_len, buf);

        received_data_len = ringbuffer_data(svr->m_ringbuf, conn->m_rb, pkg_data_len, 0, &buf);
        if (pkg_data_len > received_data_len) return 0; /*数据包不完整*/

        /*完整的数据包不在一个块内*/
        if (buf == NULL) buf = center_svr_conn_merge_rb(conn);
        if (buf == NULL) return -1;

        /*解包并移除已经获取的数据*/
        pkg = center_svr_conn_decode_pkg_buf(
            conn, &pkg_decode_len,
            svr->m_pkg_meta, buf + sizeof(pkg_data_len), pkg_data_len - sizeof(pkg_data_len));
        conn->m_rb = ringbuffer_yield(svr->m_ringbuf, conn->m_rb, pkg_data_len);

        if (pkg) {
            if (conn->m_svr->m_debug) {
                CPE_INFO(
                    conn->m_svr->m_em,
                    "%s: set %d: conn %d: <== recv one pkg (net-size=%d, data-size=%d)\n%s",
                    center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd, (int)pkg_data_len, (int)pkg_decode_len,
                    dr_json_dump_inline(&svr->m_dump_buffer, pkg, pkg_decode_len, svr->m_pkg_meta));
            }

            if (pkg->cmd >= sizeof(g_center_cli_ops) / sizeof(g_center_cli_ops[0])
                || g_center_cli_ops[pkg->cmd] == NULL)
            {
                CPE_ERROR(
                    svr->m_em, "%s: set %d: conn %d: cmd %d: no processor of cmd!",
                    center_svr_name(svr), conn->m_set ? conn->m_set->m_set->id : 0, conn->m_fd, pkg->cmd);
                return 0;
            }

            (*g_center_cli_ops[pkg->cmd])(conn, pkg, pkg_decode_len);
       }

        /*清理可能的解包缓存*/
        if (conn->m_tb) { 
            ringbuffer_free(svr->m_ringbuf, conn->m_tb);
            conn->m_tb = NULL;
        }
    }

    return 0;
}
