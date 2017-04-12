#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "set_svr_center_fsm.h"

static int set_svr_center_process_data(set_svr_center_t center);

void set_svr_center_rw_cb(EV_P_ ev_io *w, int revents) {
    set_svr_center_t center = w->data;
    set_svr_t svr = center->m_svr;
    char * buffer;

    if (revents & EV_READ) {
        ringbuffer_block_t blk;

        blk = set_svr_ringbuffer_alloc(svr, center->m_read_block_size, center->m_conn_id);
        if (blk == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: center: recv not enouth ringbuf, len=%d!",
                set_svr_name(svr), (int)center->m_read_block_size);
            set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
            return;
        }

        buffer = NULL;
        ringbuffer_block_data(svr->m_ringbuf, blk, 0, (void **)&buffer);
        assert(buffer);

        for(;;) {
            int bytes = cpe_recv(center->m_fd, buffer, center->m_read_block_size, 0);
            if (bytes > 0) {
                if (svr->m_debug >= 2) {
                    CPE_INFO(
                        svr->m_em, "%s: center: recv %d bytes data!",
                        set_svr_name(svr), bytes);
                }

                ringbuffer_shrink(svr->m_ringbuf, blk, bytes);
                set_svr_center_link_node_r(center, blk);
                break;
            }
            else if (bytes == 0) {
                blk = ringbuffer_yield(svr->m_ringbuf, blk, center->m_read_block_size);
                assert(blk == NULL);
                CPE_ERROR(svr->m_em, "%s: center: free for recv return 0!", set_svr_name(svr));
                set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
                return;
            }
            else {
                assert(bytes == -1);

                switch(errno) {
                case EWOULDBLOCK:
                case EINPROGRESS:
                    blk = ringbuffer_yield(svr->m_ringbuf, blk, center->m_read_block_size);
                    assert(blk == NULL);
                    break;
                case EINTR:
                    continue;
                default:
                    blk = ringbuffer_yield(svr->m_ringbuf, blk, center->m_read_block_size);
                    assert(blk == NULL);
                    CPE_ERROR(
                        svr->m_em, "%s: center: free for recv error, errno=%d (%s)!",
                        set_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                    set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
                    return;
                }
            }
        }

        if (set_svr_center_process_data(center) != 0) return;
    }

    if (revents & EV_WRITE) {
        while(center->m_wb) {
            void * data;
            int block_size;
            int bytes;

            block_size = ringbuffer_block_data(svr->m_ringbuf, center->m_wb, 0, &data);
            assert(block_size > 0);
            assert(data);

            bytes = cpe_send(center->m_fd, data, block_size, 0);
            if (bytes > 0) {
                if (svr->m_debug >= 2) {
                    CPE_INFO(
                        svr->m_em, "%s: center: send %d bytes data!",
                        set_svr_name(svr), bytes);
                }

                center->m_wb = ringbuffer_yield(svr->m_ringbuf, center->m_wb, bytes);
                if (bytes < block_size) break;
            }
            else if (bytes == 0) {
                CPE_ERROR(
                    svr->m_em, "%s: center: free for send return 0!",
                    set_svr_name(svr));
                set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
                return;
            }
            else {
                int err = cpe_sock_errno();
                assert(bytes == -1);

                if (err == EWOULDBLOCK || err == EINPROGRESS) break;
                if (err == EINTR) continue;

                CPE_ERROR(
                    svr->m_em, "%s: center: free for send error, errno=%d (%s)!",
                    set_svr_name(svr), err, cpe_sock_errstr(err));

                set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);

                return;
            }
        }
    }

    ev_io_stop(svr->m_ev_loop, &center->m_watcher);
    set_svr_center_start_watch(center);
}

static void * set_svr_center_merge_rb(set_svr_center_t center) {
    set_svr_t svr = center->m_svr;
    int length = ringbuffer_block_total_len(svr->m_ringbuf, center->m_rb);
    ringbuffer_block_t new_blk;
    void * buf;

    new_blk = set_svr_ringbuffer_alloc(svr, length, center->m_conn_id);
    if (new_blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: center: recv: not enouth ringbuf, len=%d!",
            set_svr_name(svr), (int)length);
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
        return NULL;
    }

    buf = ringbuffer_copy(svr->m_ringbuf, center->m_rb, 0, new_blk);
    assert(buf);

    ringbuffer_free(svr->m_ringbuf, center->m_rb);
    new_blk->id = center->m_conn_id;
    center->m_rb = new_blk;

    return buf;
}

static void * set_svr_center_decode_pkg_buf(set_svr_center_t center, size_t * decode_len, LPDRMETA meta, void const * data, size_t data_len) {
    set_svr_t svr = center->m_svr;
    size_t curent_pkg_size = 2048;
    void * buf;
    int decode_size;

    while(curent_pkg_size < data_len) { 
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    if (center->m_tb) ringbuffer_free(svr->m_ringbuf, center->m_tb);

    center->m_tb = set_svr_ringbuffer_alloc(svr, curent_pkg_size, center->m_conn_id);
    if (center->m_tb == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: decode: not enouth ringbuf, to decode pkg, data-len=%d, require-buf-len=%d!",
            set_svr_name(svr), (int)data_len, (int)curent_pkg_size);
        return NULL;
    }
    center->m_tb->id = center->m_fd;

    buf = NULL;
    ringbuffer_block_data(svr->m_ringbuf, center->m_tb, 0, &buf);
    assert(buf);
    bzero(buf, curent_pkg_size);

    decode_size = dr_pbuf_read(buf, curent_pkg_size, data, data_len, meta, svr->m_em);
    if (decode_size < 0) {
        if (decode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size < center->m_max_pkg_size) {
                curent_pkg_size *= 2;
                if (curent_pkg_size > center->m_max_pkg_size) curent_pkg_size = center->m_max_pkg_size;
                
                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: decode: decode require buf not enouth, resize to %d, input_data_len=%d",
                        set_svr_name(svr), (int)curent_pkg_size, (int)data_len);
                }

                goto RESIZE_AND_TRY_AGAIN;
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: decode: decode require buf too big!, curent_pkg_size=%d, input_data_len=%d",
                    set_svr_name(svr), (int)curent_pkg_size, (int)data_len);
                return NULL;
            }
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: decode: decode fail!, curent_pkg_size=%d, input_data_len=%d",
                set_svr_name(svr), (int)curent_pkg_size, (int)data_len);
            return NULL;
        }
    }

    if (decode_len) *decode_len = decode_size;

    return buf;
}

static int set_svr_center_process_data(set_svr_center_t center) {
    set_svr_t svr = center->m_svr;
    void * buf;
    int received_data_len;
    uint32_t pkg_data_len;
    size_t pkg_decode_len;
    SVR_CENTER_PKG * pkg;

    while(center->m_rb) {
        received_data_len = ringbuffer_data(svr->m_ringbuf, center->m_rb, sizeof(pkg_data_len), 0, &buf);
        if (received_data_len < sizeof(pkg_data_len)) return 0; /*缓存数据不够读取包长度*/

         /*数据主够读取包的大小，但是头块太小，无法保存数据头，提前合并一次数据*/
        if (buf == NULL) buf = set_svr_center_merge_rb(center);
        if (buf == NULL) return -1;

        CPE_COPY_HTON32(&pkg_data_len, buf);

        received_data_len = ringbuffer_data(svr->m_ringbuf, center->m_rb, pkg_data_len, 0, &buf);
        if (pkg_data_len > received_data_len) return 0; /*数据包不完整*/


        ringbuffer_data(svr->m_ringbuf, center->m_rb, pkg_data_len, 0, &buf);

        /*完整的数据包不在一个块内*/
        if (buf == NULL) buf = set_svr_center_merge_rb(center);
        if (buf == NULL) return -1;

        /*解包并移除已经获取的数据*/
        pkg = set_svr_center_decode_pkg_buf(
            center, &pkg_decode_len,
            center->m_pkg_meta, buf + sizeof(pkg_data_len), pkg_data_len - sizeof(pkg_data_len));
        center->m_rb = ringbuffer_yield(svr->m_ringbuf, center->m_rb, pkg_data_len);

        if (pkg) {
            if (center->m_svr->m_debug) {
                CPE_INFO(
                    center->m_svr->m_em,
                    "%s: center: <== recv one pkg (net-size=%d, data-size=%d)\n%s",
                    set_svr_name(center->m_svr), (int)pkg_data_len, (int)pkg_decode_len,
                    dr_json_dump_inline(&svr->m_dump_buffer_body, pkg, pkg_decode_len, center->m_pkg_meta));
            }

            set_svr_center_apply_pkg(center, pkg);
        }

        /*清理可能的解包缓存*/
        if (center->m_tb) { 
            ringbuffer_free(svr->m_ringbuf, center->m_tb);
            center->m_tb = NULL;
        }
    }

    return 0;
}
