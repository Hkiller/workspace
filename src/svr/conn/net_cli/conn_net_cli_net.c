#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_pbuf.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "svr/conn/net_cli/conn_net_cli_pkg.h"
#include "svr/conn/net_cli/conn_net_cli_svr_stub.h"
#include "protocol/svr/conn/svr_conn_net.h"
#include "conn_net_cli_internal_ops.h"

static int conn_net_cli_process_data(conn_net_cli_t cli);

void conn_net_cli_rw_cb(EV_P_ ev_io *w, int revents) {
    conn_net_cli_t cli;
    char * buffer;

    cli = w->data;

    if (revents & EV_READ) {
        ringbuffer_block_t blk;

        blk = ringbuffer_alloc(cli->m_ringbuf , cli->m_read_block_size);
        if (blk == NULL) {
            CPE_ERROR(
                cli->m_em, "%s: recv: not enouth ringbuf, len=%d!",
                conn_net_cli_name(cli), (int)cli->m_read_block_size);
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
            return;
        }

        buffer = NULL;
        ringbuffer_block_data(cli->m_ringbuf, blk, 0, (void **)&buffer);
        assert(buffer);

        for(;;) {
            ssize_t bytes = cpe_recv(cli->m_fd, buffer, cli->m_read_block_size, 0);

            if (bytes > 0) {
                if (cli->m_debug >= 2) {
                    CPE_INFO(
                        cli->m_em, "%s: recv %d bytes data!",
                        conn_net_cli_name(cli), (int)bytes);
                }

                ringbuffer_shrink(cli->m_ringbuf, blk, (int)bytes);
                conn_net_cli_link_node_r(cli, blk);
                break;
            }
            else if (bytes == 0) {
                blk = ringbuffer_yield(cli->m_ringbuf, blk, cli->m_read_block_size);
                assert(blk == NULL);
                CPE_ERROR(cli->m_em, "%s: free for recv return 0!", conn_net_cli_name(cli));
                conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
                return;
            }
            else {
                assert(bytes == -1);

                switch(errno) {
                case EINPROGRESS:
                case EWOULDBLOCK:
                    blk = ringbuffer_yield(cli->m_ringbuf, blk, cli->m_read_block_size);
                    assert(blk == NULL);
                    break;
                case EINTR:
                    continue;
                default:
                    blk = ringbuffer_yield(cli->m_ringbuf, blk, cli->m_read_block_size);
                    assert(blk == NULL);
                    CPE_ERROR(
                        cli->m_em, "%s: free for recv error, errno=%d (%s)!",
                        conn_net_cli_name(cli), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                    conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
                    return;
                }
            }
        }

        if (conn_net_cli_process_data(cli) != 0) return;
    }

    if (revents & EV_WRITE) {
        while(cli->m_wb) {
            void * data;
            int block_size;
            int bytes;

            block_size = ringbuffer_block_data(cli->m_ringbuf, cli->m_wb, 0, &data);
            assert(block_size > 0);
            assert(data);

            bytes = (int)cpe_send(cli->m_fd, data, block_size, 0);
            if (bytes > 0) {
                if (cli->m_debug >= 2) {
                    CPE_INFO(
                        cli->m_em, "%s: send %d bytes data!",
                        conn_net_cli_name(cli), bytes);
                }

                cli->m_wb = ringbuffer_yield(cli->m_ringbuf, cli->m_wb, bytes);

                if (bytes < block_size) break;
            }
            else if (bytes == 0) {
                CPE_ERROR(
                    cli->m_em, "%s: free for send return 0!",
                    conn_net_cli_name(cli));
                conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
                return;
            }
            else {
                int err = cpe_sock_errno();
                assert(bytes == -1);

                if (err == EWOULDBLOCK) break;
                if (err == EINTR) continue;

                CPE_ERROR(
                    cli->m_em, "%s: free for send error, errno=%d (%s)!",
                    conn_net_cli_name(cli), err, cpe_sock_errstr(err));

                conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);

                return;
            }
        }
    }

    if (cli->m_fd >= 0) {
        ev_io_stop(cli->m_ev_loop, &cli->m_watcher);
        conn_net_cli_start_watch(cli);
    }
}

static void * conn_net_cli_merge_rb(conn_net_cli_t cli) {
    int length = ringbuffer_block_total_len(cli->m_ringbuf, cli->m_rb);
    ringbuffer_block_t new_blk;
    void * buf;

    new_blk = ringbuffer_alloc(cli->m_ringbuf, length);
    if (new_blk == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: recv: not enouth ringbuf, len=%d!",
            conn_net_cli_name(cli), (int)length);
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
        return NULL;
    }

    buf = ringbuffer_copy(cli->m_ringbuf, cli->m_rb, 0, new_blk);
    assert(buf);

    ringbuffer_free(cli->m_ringbuf, cli->m_rb);
    cli->m_rb = new_blk;

    return buf;
}

static int conn_net_cli_decode_pkg_buf(conn_net_cli_t cli, LPDRMETA meta, void const * data, size_t data_len) {
    size_t curent_pkg_size = cli->m_decode_block_size;
    void * buf;
    int decode_size;

RESIZE_AND_TRY_AGAIN:
    if (cli->m_tb) ringbuffer_free(cli->m_ringbuf, cli->m_tb);

    cli->m_tb = ringbuffer_alloc(cli->m_ringbuf, (int)curent_pkg_size);
    if (cli->m_tb == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: decode: not enouth ringbuf, to decode pkg, data-len=%d, require-buf-len=%d!",
            conn_net_cli_name(cli), (int)data_len, (int)curent_pkg_size);
        return -1;
    }
    cli->m_tb->id = 3;

    buf = NULL;
    ringbuffer_data(cli->m_ringbuf, cli->m_tb, (int)curent_pkg_size, 0, &buf);
    assert(buf);
    bzero(buf, curent_pkg_size);

    decode_size = dr_pbuf_read(buf, curent_pkg_size, data, data_len, meta, cli->m_em);
    if (decode_size < 0) {
        if (decode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size < cli->m_max_pkg_size) {
                curent_pkg_size *= 2;

                if (cli->m_debug) {
                    CPE_INFO(
                        cli->m_em, "%s: decode: decode require buf not enouth, resize to %d, input_data_len=%d",
                        conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
                }

                goto RESIZE_AND_TRY_AGAIN;
            }
            else {
                CPE_ERROR(
                    cli->m_em, "%s: decode: decode require buf too big!, curent_pkg_size=%d, input_data_len=%d",
                    conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
                return -1;
            }
        }
        else {
            CPE_ERROR(
                cli->m_em, "%s: decode: decode fail!, curent_pkg_size=%d, input_data_len=%d",
                conn_net_cli_name(cli), (int)curent_pkg_size, (int)data_len);
            return -1;
        }
    }

    if (curent_pkg_size > cli->m_decode_block_size) cli->m_decode_block_size = (uint32_t)curent_pkg_size;

    dp_req_set_meta(cli->m_incoming_body, meta);
    dp_req_set_buf(cli->m_incoming_body, buf, curent_pkg_size);
    dp_req_set_size(cli->m_incoming_body, decode_size);

    return 0;
}

static int conn_net_cli_process_data(conn_net_cli_t cli) {
    void * buf;
    SVR_CONN_NET_RES_HEAD * head;
    int received_data_len;
    uint16_t pkg_data_len;
    conn_net_cli_svr_stub_t svr_stub;
    conn_net_cli_pkg_t cli_pkg = conn_net_cli_incoming_pkg(cli);
    int decode_result;

    if (cli_pkg == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: cli_pkg alloc fail!!!", conn_net_cli_name(cli));
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
        return -1;
    }

    while(cli->m_rb) {
        received_data_len = ringbuffer_data(cli->m_ringbuf, cli->m_rb, sizeof(uint16_t), 0, &buf);
        if (received_data_len < sizeof(uint16_t)) return 0; /*缓存数据不够读取包长度 */ 

         /*数据主够读取包的大小，但是头块太小，无法保存数据头，提前合并一次数据 */  
        if (buf == NULL) buf = conn_net_cli_merge_rb(cli);
        if (buf == NULL) {
            CPE_ERROR(
                cli->m_em, "%s: conn_net_cli_merge_rb fail!!!", conn_net_cli_name(cli));
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
            return -1;
        }

        CPE_COPY_HTON16(&pkg_data_len, buf);

        received_data_len = ringbuffer_data(cli->m_ringbuf, cli->m_rb, pkg_data_len, 0, &buf);
        if (pkg_data_len > received_data_len) return 0; /*数据包不完整 */  

        /*包大小太小，则断开连接 */  
        if (pkg_data_len < sizeof(SVR_CONN_NET_RES_HEAD)) {
            CPE_ERROR(
                cli->m_em, "%s: free for receive small pkg, pkg-len=%d!!!",
                conn_net_cli_name(cli), pkg_data_len);
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
            return -1;
        }

        /*完整的数据包不在一个块内 */  
        if (buf == NULL) buf = conn_net_cli_merge_rb(cli);
        if (buf == NULL) {
            CPE_ERROR(
                cli->m_em, "%s: conn_net_cli_merge_rb fail!!!", conn_net_cli_name(cli));
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
            return -1;
        }

        /*转换成内部的pkg */  
        head = buf;
        cli_pkg->m_result = head->result;
        cli_pkg->m_flags = head->flags;
        CPE_COPY_HTON16(&cli_pkg->m_svr_type, &head->from_svr);
        CPE_COPY_HTON32(&cli_pkg->m_sn, &head->sn);

        svr_stub = conn_net_cli_svr_stub_find_by_id(cli, cli_pkg->m_svr_type);
        if (svr_stub == NULL) {
            CPE_ERROR(cli->m_em, "%s: svr_stub %d is unknown!", conn_net_cli_name(cli), cli_pkg->m_svr_type);
            cli->m_rb = ringbuffer_yield(cli->m_ringbuf, cli->m_rb, pkg_data_len);
            continue;
        }

        /*解包并移除已经获取的数据 */  
        decode_result = conn_net_cli_decode_pkg_buf(cli, svr_stub->m_pkg_meta, head + 1, pkg_data_len - sizeof(SVR_CONN_NET_RES_HEAD));
        cli->m_rb = ringbuffer_yield(cli->m_ringbuf, cli->m_rb, pkg_data_len);

        if (decode_result == 0) {
            dp_req_set_parent(conn_net_cli_pkg_to_dp_req(cli_pkg), cli->m_incoming_body);

            if (cli_pkg->m_sn) {
                if (cli->m_debug) {
                    CPE_INFO(
                        cli->m_em, "%s: %s <== receive one response, sn=%d, pkg-size=%d\n%s",
                        conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub),
                        cli_pkg->m_sn, (int)pkg_data_len, dp_req_dump(cli->m_incoming_body, &cli->m_dump_buffer));
                }

                if (svr_stub->m_response_dispatch_to == NULL) {
                    CPE_ERROR(
                        cli->m_em, "%s: %s: <== receive on response, sn=%d, pkg-size=%d: no response-send-to configured!",
                        conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub), cli_pkg->m_sn, (int)pkg_data_len);
                }
                else if (dp_dispatch_by_string(svr_stub->m_response_dispatch_to, dp_req_mgr(cli->m_incoming_body), cli->m_incoming_body, cli->m_em) != 0) {
                    CPE_ERROR(
                        cli->m_em, "%s: %s: <== receive on response, sn=%d, pkg-size=%d: dispatch-to %s fail!",
                        conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub),
                        cli_pkg->m_sn, (int)pkg_data_len, cpe_hs_data(svr_stub->m_response_dispatch_to));
                }
            }
            else {
                if (cli->m_debug) {
                    CPE_INFO(
                        cli->m_em, "%s: %s <== receive one notify, pkg-size=%d\n%s",
                        conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub),
                        (int)pkg_data_len, dp_req_dump(cli->m_incoming_body, &cli->m_dump_buffer));
                }

                if (svr_stub->m_notify_dispatch_to == NULL) {
                    CPE_ERROR(
                        cli->m_em, "%s: %s: <== receive on notify, pkg-size=%d: no notify-send-to configured!",
                        conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub), (int)pkg_data_len);
                }
                else if (dp_dispatch_by_string(svr_stub->m_notify_dispatch_to, dp_req_mgr(cli->m_incoming_body), cli->m_incoming_body, cli->m_em) != 0) {
                    CPE_ERROR(
                        cli->m_em, "%s: %s: <== receive on notify, pkg-size=%d: dispatch-to %s fail!",
                        conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub),
                        (int)pkg_data_len, cpe_hs_data(svr_stub->m_notify_dispatch_to));
                }
            }

            dp_req_set_parent(conn_net_cli_pkg_to_dp_req(cli_pkg), NULL);
        }
        else {
            CPE_ERROR(
                cli->m_em, "%s: %s <== receive one pkg, decode fail!",
                conn_net_cli_name(cli), conn_net_cli_svr_stub_type_name(svr_stub));
        }

        /*清理可能的解包缓存 */  
        if (cli->m_tb) { 
            dp_req_set_buf(cli->m_incoming_body, NULL, 0);
            ringbuffer_free(cli->m_ringbuf, cli->m_tb);
            cli->m_tb = NULL;
        }
    }

    return 0;
}
