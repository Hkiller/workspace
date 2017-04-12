#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "set_svr_stub_internal_ops.h"
#include "protocol/svr/set/set_share_pkg.h"

static dp_req_t set_svr_stub_incoming_pkg_buf(set_svr_stub_t stub) {
    if (stub->m_incoming_buf == NULL) {
        stub->m_incoming_buf = dp_req_create(gd_app_dp_mgr(stub->m_app), 0);
        if (stub->m_incoming_buf == NULL) {
            CPE_ERROR(stub->m_em, "%s: crate incoming buf fail!", set_svr_stub_name(stub));
            return NULL;
        }
    }

    return stub->m_incoming_buf;
}

ptr_int_t set_svr_stub_tick(void * ctx, ptr_int_t arg, float delta) {
    set_svr_stub_t stub = ctx;
    int process_count = 0;
    int rv;
    dp_req_t body = NULL;
    dp_req_t head;
    dp_req_t carry;
    SET_PKG_HEAD * head_buf;

    for(process_count = 0; process_count < stub->m_process_count_per_tick; ++process_count) {
        if (body == NULL) {
            body = set_svr_stub_incoming_pkg_buf(stub);
            if (body == NULL) break;
        }

        rv = set_chanel_r_peak(stub->m_chanel, body, stub->m_em);
        if (rv != 0) {
            if (rv == set_chanel_error_chanel_empty) break;

            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: <== peak pkg fail, error=%d (%s)!",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                rv, set_chanel_str_error(rv));
            goto NEXT_PKG;
        }

        head = set_pkg_head_find(body);
        assert(head);
        head_buf = dp_req_data(head);

        carry = set_pkg_carry_find(body);
        assert(carry);

        /*检查目的地址*/
        if (head_buf->to_svr_type != stub->m_svr_type->m_svr_type_id || (head_buf->to_svr_id != 0 && head_buf->to_svr_id != stub->m_svr_id)) {
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d : to_svr %d.%d mismatch!",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->from_svr_type, head_buf->from_svr_id,
                head_buf->to_svr_type, head_buf->to_svr_id);
            goto NEXT_PKG;
        }

        /*根据包类型获取包所属的服务类型*/
        switch(set_pkg_category(head)) {
        case set_pkg_request:
            dp_req_set_meta(body, stub->m_svr_type->m_pkg_meta);
            break;
        case set_pkg_response:
        case set_pkg_notify: {
            /*找到源服务信息*/
            set_svr_svr_info_t from_svr_type = set_svr_svr_info_find(stub, head_buf->from_svr_type);
            if (from_svr_type == NULL) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: from svr type %d is unknown!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id, head_buf->from_svr_type);
                goto NEXT_PKG;
            }

            dp_req_set_meta(body, from_svr_type->m_pkg_meta);
            break;
        }
        default:
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: pkg category %d is unknown!",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->from_svr_type, head_buf->from_svr_id,
                (int)set_pkg_category(head));
            goto NEXT_PKG;
        }

        if (stub->m_debug) {
            if (set_pkg_pack_state(head) == set_pkg_packed) {
                CPE_INFO(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d:\n\thead: %s\tcarry: %s\tbody: [packed %d bytes]",
                    set_svr_stub_name(stub),
                    stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id,
                    dp_req_dump(head, &stub->m_dump_buffer_head),
                    dp_req_dump(carry, &stub->m_dump_buffer_carry),
                    (int)dp_req_size(body));
            }
            else {
                CPE_INFO(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d:\n\thead: %s\tcarry: %s\tbody: %s",
                    set_svr_stub_name(stub),
                    stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id,
                    dp_req_dump(head, &stub->m_dump_buffer_head),
                    dp_req_dump(carry, &stub->m_dump_buffer_carry),
                    dp_req_dump(body, &stub->m_dump_buffer_body));
            }
        }

        switch(set_pkg_category(head)) {
        case set_pkg_request: {
            if (stub->m_request_dispatch_to == NULL) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: request-dispatch-to configured!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id);
                goto NEXT_PKG;
            }

            if (dp_dispatch_by_string(stub->m_request_dispatch_to, dp_req_mgr(body), body, stub->m_em) != 0) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: request-dispatch-to %s fail!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id,
                    cpe_hs_data(stub->m_request_dispatch_to));
                goto NEXT_PKG;
            }
            break;
        }
        case set_pkg_response: {
            set_svr_svr_info_t dispatch_info = set_svr_svr_info_find(stub, head_buf->from_svr_type);
            cpe_hash_string_t dispatch_to = dispatch_info ? dispatch_info->m_response_dispatch_to : NULL;
            if (dispatch_to == NULL) dispatch_to = stub->m_response_dispatch_to;

            if (dispatch_to == NULL) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: response-dispatch-to not configured!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id);
                goto NEXT_PKG;
            }

            if (dp_dispatch_by_string(dispatch_to, dp_req_mgr(body), body, stub->m_em) != 0) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: response-dispatch-to %s fail!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id,
                    cpe_hs_data(dispatch_to));
                goto NEXT_PKG;
            }

            break;
        }
        case set_pkg_notify: {
            set_svr_svr_info_t dispatch_info = set_svr_svr_info_find(stub, head_buf->from_svr_type);
            cpe_hash_string_t dispatch_to = dispatch_info ? dispatch_info->m_notify_dispatch_to : NULL;
            if (dispatch_to == NULL) dispatch_to = stub->m_notify_dispatch_to;
 
            if (dispatch_to == NULL) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: notify-dispatch-to not configured!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id);
                goto NEXT_PKG;
            }

            if (dp_dispatch_by_string(dispatch_to, dp_req_mgr(body), body, stub->m_em) != 0) {
                CPE_ERROR(
                    stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: notify-dispatch-to %s fail!",
                    set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                    head_buf->from_svr_type, head_buf->from_svr_id,
                    cpe_hs_data(dispatch_to));
                goto NEXT_PKG;
            }
            break;
        }
        default:
            CPE_ERROR(
                stub->m_em, "%s: svr %s.%d: <== recv one pkg from %d.%d: pkg category is unknown!",
                set_svr_stub_name(stub), stub->m_svr_type->m_svr_type_name, stub->m_svr_id,
                head_buf->from_svr_type, head_buf->from_svr_id);
            break;
        }

    NEXT_PKG:
        rv = set_chanel_r_erase(stub->m_chanel, stub->m_em);
    }

    return process_count;
}
