#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_request.h"
#include "svr/set/share/set_pkg.h"
#include "protocol/svr/conn/svr_conn_net.h"
#include "conn_svr_ops.h"

static int conn_svr_conn_process_data(conn_svr_t svr, conn_svr_conn_t conn);

void conn_svr_listener_cb(EV_P_ ev_io *w, int revents) {
    conn_svr_t svr;
    conn_svr_conn_t conn;
    int new_fd;
    uint32_t new_conn_id;

    svr = w->data;
    assert(svr);

    new_fd = cpe_accept(svr->m_fd, 0, 0);
    if (new_fd == -1) {
        CPE_ERROR(
            svr->m_em, "%s: accept error, errno=%d (%s)",
            conn_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return;
    }

    new_conn_id = svr->m_conn_max_id + 1;

    conn = conn_svr_conn_create(svr, new_fd, new_conn_id);
    if (conn == NULL) {
        CPE_ERROR(svr->m_em, "%s: create conn fail!", conn_svr_name(svr));
        cpe_sock_close(new_fd);
        return;
    }

    svr->m_conn_max_id = new_conn_id;
}

void conn_svr_rw_cb(EV_P_ ev_io *w, int revents) {
    conn_svr_t svr;
    conn_svr_conn_t conn;
    char * buffer;

    conn = w->data;
    svr = conn->m_svr;

    conn_svr_conn_update_op_time(conn);

    if (revents & EV_READ) {
        ringbuffer_block_t blk;
        if (conn_svr_conn_alloc(&blk, svr, conn, svr->m_read_block_size) != 0) return;
        assert(blk);

        buffer = (char *)(blk + 1);

        for(;;) {
            int bytes = cpe_recv(conn->m_fd, buffer, svr->m_read_block_size, 0);
            if (bytes > 0) {
                if (svr->m_debug >= 2) {
                    CPE_INFO(
                        svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): recv %d bytes data!",
                        conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, bytes);
                }

                ringbuffer_shrink(svr->m_ringbuf, blk, bytes);
                conn_svr_conn_link_node_r(conn, blk);
                break;
            }
            else if (bytes == 0) {
                ringbuffer_shrink(svr->m_ringbuf, blk, 0);
                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free for recv return 0!",
                        conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
                }
                conn_svr_conn_free(conn);
                return;
            }
            else {
                assert(bytes == -1);

                switch(errno) {
                case EWOULDBLOCK:
                case EINPROGRESS:
                    ringbuffer_shrink(svr->m_ringbuf, blk, 0);
                    break;
                case EINTR:
                    continue;
                default:
                    ringbuffer_shrink(svr->m_ringbuf, blk, 0);
                    CPE_ERROR(
                        svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free for recv error, errno=%d (%s)!",
                        conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                    conn_svr_conn_free(conn);
                    return;
                }
            }
        }

        if (conn_svr_conn_process_data(svr, conn) != 0) return;
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
                        svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): send %d bytes data!",
                        conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, bytes);
                }

                conn->m_wb = ringbuffer_yield(svr->m_ringbuf, conn->m_wb, bytes);
                if (bytes < block_size) break;
            }
            else if (bytes == 0) {
                CPE_ERROR(
                    svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free for send return 0!",
                    conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
                conn_svr_conn_free(conn);
                return;
            }
            else {
                int err = cpe_sock_errno();
                assert(bytes == -1);

                if (err == EWOULDBLOCK || err == EINPROGRESS) break;
                if (err == EINTR) continue;

                CPE_ERROR(
                    svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free for send error, errno=%d (%s)!",
                    conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, err, cpe_sock_errstr(err));
                conn_svr_conn_free(conn);
                return;
            }
        }
    }

    ev_io_stop(svr->m_ev_loop, &conn->m_watcher);
    conn_svr_conn_start_watch(conn);
}

static void * conn_svr_conn_merge_rb(conn_svr_t svr, conn_svr_conn_t conn) {
    int length = ringbuffer_block_total_len(svr->m_ringbuf, conn->m_rb);
    ringbuffer_block_t new_blk;
    void * buf;

    if (conn_svr_conn_alloc(&new_blk, svr, conn, length) != 0) return NULL;
    assert(new_blk);

    buf = ringbuffer_copy(svr->m_ringbuf, conn->m_rb, 0, new_blk);
    assert(buf);

    ringbuffer_free(svr->m_ringbuf, conn->m_rb);
    conn->m_rb = new_blk;

    return buf;
}

static int conn_svr_conn_process_data(conn_svr_t svr, conn_svr_conn_t conn) {
    void * buf;
    SVR_CONN_NET_REQ_HEAD * head;
    int received_data_len;
    uint16_t pkg_data_len;
    uint16_t to_svr_type;
    uint32_t sn;
    uint16_t ss_pkg_len;
    conn_svr_backend_t backend;
    dp_req_t ss_pkg = NULL;
    dp_req_t ss_pkg_head = NULL;
    dp_req_t ss_pkg_carry = NULL;
    CONN_SVR_CONN_INFO * ss_pkg_carry_data;

    while(conn->m_rb) {
        received_data_len = ringbuffer_data(svr->m_ringbuf, conn->m_rb, sizeof(uint16_t), 0, &buf);
        if (received_data_len < sizeof(uint16_t)) return 0; /*缓存数据不够读取包长度*/

         /*数据主够读取包的大小，但是头块太小，无法保存数据头，提前合并一次数据*/
        if (buf == NULL) buf = conn_svr_conn_merge_rb(svr, conn);
        if (buf == NULL) return -1;

        CPE_COPY_NTOH16(&pkg_data_len, buf);

        received_data_len = ringbuffer_data(svr->m_ringbuf, conn->m_rb, pkg_data_len, 0, &buf);

        if (pkg_data_len > received_data_len) return 0; /*数据包不完整*/

        /*包大小太小，则断开连接*/
        if (pkg_data_len < sizeof(SVR_CONN_NET_REQ_HEAD)) {
            CPE_ERROR(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free for receive small pkg, pkg-len=%d!!!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, pkg_data_len);
            conn_svr_conn_free(conn);
            return -1;
        }

        ringbuffer_data(svr->m_ringbuf, conn->m_rb, pkg_data_len, 0, &buf);

        /*完整的数据包不在一个块内*/
        if (buf == NULL) buf = conn_svr_conn_merge_rb(svr, conn);
        if (buf == NULL) return -1;

        /*转换成内部的pkg*/
        head = buf;
        CPE_COPY_NTOH16(&to_svr_type, &head->to_svr);
        CPE_COPY_NTOH32(&sn, &head->sn);

        if (ss_pkg == NULL) {
            ss_pkg = conn_svr_pkg_buf(svr);
            if (ss_pkg == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): free for alloc ss_pkg fail, pkg-len=%d!!!",
                    conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, ss_pkg_len);
                conn_svr_conn_free(conn);
                return -1;
            }

            ss_pkg_head = set_pkg_head_find(ss_pkg);
            assert(ss_pkg_head);

            ss_pkg_carry = set_pkg_carry_find(ss_pkg);
            assert(ss_pkg_carry);
        }

        ss_pkg_len = pkg_data_len - sizeof(SVR_CONN_NET_REQ_HEAD);

        /*设置head*/
        set_pkg_init(ss_pkg_head);
        set_pkg_set_to_svr(ss_pkg_head, to_svr_type, 0);
        set_pkg_set_sn(ss_pkg_head, sn);
        set_pkg_set_category(ss_pkg_head, set_pkg_request);
        set_pkg_set_pack_state(ss_pkg_head, set_pkg_packed);

        /*设置carry*/
        ss_pkg_carry_data = set_pkg_carry_data(ss_pkg_carry);
        *ss_pkg_carry_data = conn->m_data;

        /*查找需要发送的目的服务信息*/
        backend = conn_svr_backend_find(svr, to_svr_type);
        if (backend == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): to_svr %d is unknown!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, to_svr_type);
            conn->m_rb = ringbuffer_yield(svr->m_ringbuf, conn->m_rb, pkg_data_len);
            if (conn_svr_conn_net_send(conn, svr->m_conn_svr_type, SVR_CONN_NET_ERROR_SVR_UNKNOWN, sn, NULL, 0, NULL) != 0) return -1;
            continue;
        }

        /*检查发送目的地的安全策略*/
        switch(backend->m_safe_policy) {
        case conn_svr_backend_any:
            break;
        case conn_svr_backend_auth_success:
            if (conn->m_auth == 0) {
                CPE_ERROR(
                    svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): to_svr %d connection not auth!",
                    conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, to_svr_type);
                conn->m_rb = ringbuffer_yield(svr->m_ringbuf, conn->m_rb, pkg_data_len);
                if (conn_svr_conn_net_send(conn, svr->m_conn_svr_type, SVR_CONN_NET_ERROR_NOT_AUTH, sn, NULL, 0, NULL) != 0) return -1;
                continue;
            }
            break;
        case conn_svr_backend_user_bind:
            if (conn->m_data.user_id == 0) {
                CPE_ERROR(
                    svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): to_svr %d connection not bind user!",
                    conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, to_svr_type);
                conn->m_rb = ringbuffer_yield(svr->m_ringbuf, conn->m_rb, pkg_data_len);
                if (conn_svr_conn_net_send(conn, svr->m_conn_svr_type, SVR_CONN_NET_ERROR_NOT_LOGIN, sn, NULL, 0, NULL) != 0) return -1;
                continue;
            }
            break;
        default:
            CPE_ERROR(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): to_svr %d safe policy %d is unknown!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, to_svr_type, backend->m_safe_policy);
            conn->m_rb = ringbuffer_yield(svr->m_ringbuf, conn->m_rb, pkg_data_len);
            if (conn_svr_conn_net_send(conn, svr->m_conn_svr_type, SVR_CONN_NET_ERROR_INTERNAL, sn, NULL, 0, NULL) != 0) return -1;
            continue;
        }

        /*设置body*/
        dp_req_set_meta(ss_pkg, NULL);
        dp_req_set_buf(ss_pkg, head + 1, ss_pkg_len);
        dp_req_set_size(ss_pkg, ss_pkg_len);

        if (conn_svr_send_pkg(svr, ss_pkg) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): send ss_pkg fail!",
                conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id);
        }

        conn->m_rb = ringbuffer_yield(svr->m_ringbuf, conn->m_rb, pkg_data_len);
    }

    return 0;
}

