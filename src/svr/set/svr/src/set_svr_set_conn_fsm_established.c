#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/share/set_chanel.h"
#include "set_svr_set_conn_fsm.h"
#include "set_svr_svr_ins.h"
#include "protocol/svr/set/set_share_pkg.h"

void set_svr_set_conn_established_rw_cb(EV_P_ ev_io *w, int revents);

static void set_svr_set_conn_fsm_established_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_t svr = conn->m_svr;
    
    assert(conn->m_fd != -1);
    ev_io_init(&conn->m_watcher, set_svr_set_conn_established_rw_cb, conn->m_fd, conn->m_wb ? EV_READ | EV_WRITE : EV_READ);
    ev_io_start(svr->m_ev_loop, &conn->m_watcher);
}

static void set_svr_set_conn_fsm_established_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);

    ev_io_stop(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

static uint32_t set_svr_set_conn_fsm_established_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_t svr = conn->m_svr;
    struct set_svr_set_conn_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_set_conn_fsm_evt_disconnected:
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;
    case set_svr_set_conn_fsm_evt_wb_update:
        ev_io_stop(conn->m_svr->m_ev_loop, &conn->m_watcher);
        ev_io_init(&conn->m_watcher, set_svr_set_conn_established_rw_cb, conn->m_fd, conn->m_wb ? EV_READ | EV_WRITE : EV_READ);
        ev_io_start(svr->m_ev_loop, &conn->m_watcher);
        return FSM_KEEP_STATE;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_set_conn_fsm_create_established(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "established", set_svr_set_conn_state_established);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_established: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_set_conn_fsm_established_enter, set_svr_set_conn_fsm_established_leave);

    if (fsm_def_state_add_transition(s, set_svr_set_conn_fsm_established_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_established: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static int set_svr_set_conn_established_process_data(set_svr_set_conn_t set_conn);

void set_svr_set_conn_established_rw_cb(EV_P_ ev_io *w, int revents) {
    set_svr_set_conn_t conn = w->data;
    set_svr_t svr = conn->m_svr;

    if (revents & EV_READ) {
        if (set_svr_set_conn_read_from_net(conn, svr->m_set_read_block_size) != 0) {
            set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
            return;
        }

        if (set_svr_set_conn_established_process_data(conn) != 0) {
            set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
            return;
        }
    }

    if (revents & EV_WRITE) {
        if (set_svr_set_conn_write_to_net(conn) != 0) {
            set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
            return;
        }
    }

    set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_wb_update);
}

static int set_svr_set_conn_established_decode_pkg(set_svr_set_conn_t conn, dp_req_t body, void * data, size_t data_len, LPDRMETA meta) {
    set_svr_set_t set = conn->m_set;
    set_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = 2048;
    char * buf;
    int decode_size;

    while(curent_pkg_size < data_len) {
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    if (conn->m_tb) ringbuffer_free(svr->m_ringbuf, conn->m_tb);

    conn->m_tb = set_svr_ringbuffer_alloc(svr, curent_pkg_size, conn->m_conn_id);
    if (conn->m_tb == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: set %s: decode: not enouth ringbuf, to decode pkg, data-len=%d, require-buf-len=%d!",
            set_svr_name(svr), conn->m_fd, set_svr_set_name(set),
            (int)data_len, (int)curent_pkg_size);
        return -1;
    }
    conn->m_tb->id = conn->m_conn_id;

    buf = NULL;
    ringbuffer_block_data(svr->m_ringbuf, conn->m_tb, 0, (void**)&buf);
    assert(buf);
    bzero(buf, curent_pkg_size);

    decode_size = dr_pbuf_read(buf, curent_pkg_size, data, data_len, meta, svr->m_em);
    if (decode_size < 0) {
        if (decode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size < svr->m_set_max_pkg_size) {
                curent_pkg_size *= 2;
                if (curent_pkg_size > svr->m_set_max_pkg_size) curent_pkg_size = svr->m_set_max_pkg_size;

                if (svr->m_debug) {
                    CPE_INFO(
                        svr->m_em, "%s: conn %d: set %s: decode: decode require buf not enouth, resize to %d, input_data_len=%d",
                        set_svr_name(svr), conn->m_fd, set_svr_set_name(set),
                        (int)curent_pkg_size, (int)data_len);
                }

                goto RESIZE_AND_TRY_AGAIN;
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: set %s: decode: decode require buf too big!, curent_pkg_size=%d, input_data_len=%d",
                    set_svr_name(svr), conn->m_fd, set_svr_set_name(set),
                    (int)curent_pkg_size, (int)data_len);
                return -1;
            }
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: decode: decode fail!, curent_pkg_size=%d, input_data_len=%d",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set),
                (int)curent_pkg_size, (int)data_len);
            return -1;
        }
    }

    dp_req_set_buf(body, buf, curent_pkg_size);
    dp_req_set_size(body, decode_size);
    dp_req_set_meta(body, meta);

    return 0;
}

static int set_svr_set_conn_established_process_data(set_svr_set_conn_t conn) {
    set_svr_t svr = conn->m_svr;
    set_svr_set_t set = conn->m_set;
    void * buf;
    int received_data_len;
    uint32_t pkg_data_len;
    uint32_t left_data_len;
    dp_req_t body = NULL;
    dp_req_t head;
    dp_req_t carry;
    int rv;
    SET_PKG_HEAD * head_buf;
    set_svr_svr_ins_t to_svr_ins;
    LPDRMETA pkg_meta;

    while(conn->m_rb) {
        received_data_len = set_svr_set_conn_r_buf(conn, sizeof(pkg_data_len), &buf);
        if (received_data_len < 0) return -1;

        if (received_data_len < sizeof(pkg_data_len)) break; /*缓存数据不够读取包长度*/
        assert(buf);

        CPE_COPY_HTON32(&pkg_data_len, buf);

        if (pkg_data_len < (sizeof(SET_PKG_HEAD) + 1)) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: receive small pkg, pkg_data_len=%d, atleast %d!",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set), pkg_data_len, (int)(sizeof(SET_PKG_HEAD) + 1));
            return -1;
        }

        received_data_len = set_svr_set_conn_r_buf(conn, pkg_data_len, &buf);
        if (received_data_len < pkg_data_len) break; /*数据包不完整*/
        assert(buf);

        /*掠过包大小字段*/
        buf += sizeof(pkg_data_len);
        left_data_len = pkg_data_len - sizeof(pkg_data_len);

        /*分配缓存数据结构*/
        if (body == NULL) {
            if (svr->m_incoming_buf == NULL) {
                svr->m_incoming_buf = dp_req_create(gd_app_dp_mgr(svr->m_app), 0);
                if (svr->m_incoming_buf == NULL) {
                    CPE_ERROR(
                        svr->m_em, "%s: conn %d: set %s: alloc incoming pkg body fail!",
                        set_svr_name(svr), conn->m_fd, set_svr_set_name(set));
                    return -1;
                }
            }

            body = svr->m_incoming_buf;

            if ((head = set_pkg_head_check_create(body)) == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: set %s: alloc incoming pkg head fail!",
                    set_svr_name(svr), conn->m_fd, set_svr_set_name(set));
                dp_req_free(svr->m_incoming_buf);
                svr->m_incoming_buf = NULL;
                return -1;
            }

            if ((carry = set_pkg_carry_check_create(body, 0)) == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: set %s: alloc incoming pkg head fail!",
                    set_svr_name(svr), conn->m_fd, set_svr_set_name(set));
                dp_req_free(svr->m_incoming_buf);
                svr->m_incoming_buf = NULL;
                return -1;
            }

            head_buf = dp_req_data(head);
        }

        /*读取包头*/
        CPE_COPY_NTOH16(&head_buf->to_svr_type, buf);
        buf += 2, left_data_len-=2;
        CPE_COPY_NTOH16(&head_buf->to_svr_id, buf);
        buf += 2, left_data_len-=2;
        CPE_COPY_NTOH16(&head_buf->from_svr_type, buf);
        buf += 2, left_data_len-=2;
        CPE_COPY_NTOH16(&head_buf->from_svr_id, buf);
        buf += 2, left_data_len-=2;
        CPE_COPY_NTOH32(&head_buf->sn, buf);
        buf += 4, left_data_len-=4;
        CPE_COPY_NTOH16(&head_buf->flags, buf);
        buf += 2, left_data_len-=2;

        /*监测目标服务*/
        to_svr_ins = set_svr_svr_ins_find(svr, head_buf->to_svr_type, head_buf->to_svr_id);
        if (to_svr_ins == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: receive one pkg, to svr %d.%d not exist!",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set),
                head_buf->to_svr_type, head_buf->to_svr_id);
            set_svr_set_conn_r_erase(conn, pkg_data_len);
            continue;
        }

        if (to_svr_ins->m_set != NULL) { /*不是发送到本地服务 */
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: receive one pkg, to svr %d.%d not at local!",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set),
                head_buf->to_svr_type, head_buf->to_svr_id);
            set_svr_set_conn_r_erase(conn, pkg_data_len);
            continue;
        }

        /*根据包类型获取包所属的服务类型*/
        switch(set_pkg_category(head)) {
        case set_pkg_request:
            pkg_meta = to_svr_ins->m_svr_type->m_pkg_meta;
            assert(pkg_meta);
            break;
        case set_pkg_response:
        case set_pkg_notify: {
            set_svr_svr_type_t from_svr_type = set_svr_svr_type_find_by_id(svr, head_buf->from_svr_type);
            if (from_svr_type == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: set %s: receive one pkg, from svr type %d not exist!",
                    set_svr_name(svr), conn->m_fd, set_svr_set_name(set), head_buf->from_svr_type);
                set_svr_set_conn_r_erase(conn, pkg_data_len);
                continue;
            }
            pkg_meta = from_svr_type->m_pkg_meta;
            assert(pkg_meta);
            break;
        }
        default:
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: receive one pkg, category %d is unknown!",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set), set_pkg_category(head));
            set_svr_set_conn_r_erase(conn, pkg_data_len);
            continue;
        }

        /*读取carry*/
        if (set_pkg_carry_set_buf(carry, buf, left_data_len) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: receive small pkg(can`t containe carry), pkg_data_len=%d!",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set), pkg_data_len);
            return -1;
        }
        buf += dp_req_size(carry);
        left_data_len -= dp_req_size(carry);

        /*读取body*/
        if (set_svr_set_conn_established_decode_pkg(conn, body, buf, left_data_len, pkg_meta) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %d-%s:%d: decode incoming pkg fail, data_len=%d, pkg_meta=%s!",
                set_svr_name(svr), conn->m_fd, set->m_set_id, set->m_ip, set->m_port, 
                (int)left_data_len, dr_meta_name(pkg_meta));
            return -1;
        }

        if (conn->m_svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: conn %d: set %s: <== recv one pkg (net-size=%d)\n\thead: %s\tcarry: %s\tbody: %s",
                set_svr_name(conn->m_svr), conn->m_fd, set_svr_set_name(set),
                (int)pkg_data_len,
                dp_req_dump(head, &svr->m_dump_buffer_head),
                dp_req_dump(carry, &svr->m_dump_buffer_carry),
                dp_req_dump(body, &svr->m_dump_buffer_body));
        }

        rv = set_chanel_r_write(to_svr_ins->m_chanel, body, NULL, svr->m_em);
        if (rv != 0) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: set %s: write to chanel fail, error=%d (%s)!",
                set_svr_name(svr), conn->m_fd, set_svr_set_name(set), rv, set_chanel_str_error(rv));
            set_svr_set_conn_r_erase(conn, pkg_data_len);
            continue;
        }
        set_svr_set_conn_r_erase(conn, pkg_data_len);

        /*清理可能的解包缓存*/
        if (conn->m_tb) {
            ringbuffer_free(svr->m_ringbuf, conn->m_tb);
            conn->m_tb = NULL;
        }
    }

    return 0;
}
