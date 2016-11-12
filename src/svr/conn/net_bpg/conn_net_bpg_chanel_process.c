#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "svr/conn/net_cli/conn_net_cli_pkg.h"
#include "conn_net_bpg_internal_ops.h"

int conn_net_bpg_chanel_incoming_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_net_bpg_chanel_t chanel = ctx;
    size_t cur_data_size = 2048;
    dr_cvt_result_t decode_rv;
    size_t buf_size;

    while(cur_data_size < dp_req_size(req) * 2) {
        cur_data_size *= 2;
    }

DECODE_PKG:
    if (chanel->m_incoming_buf && dp_req_capacity(chanel->m_incoming_buf) < cur_data_size) {
        dp_req_free(chanel->m_incoming_buf);
        chanel->m_incoming_buf = NULL;
    }

    if (chanel->m_incoming_buf == NULL) {
        chanel->m_incoming_buf = bpg_pkg_create_with_body(chanel->m_bpg_pkg_manage, cur_data_size);
        if (chanel->m_incoming_buf == NULL) {
            CPE_ERROR(chanel->m_em, "%s: incoming_recv: alloc incoming_buf fail, size=%d!", conn_net_bpg_chanel_name(chanel), (int)cur_data_size);
            return -1;
        }
    }

    buf_size = dp_req_size(req);
    decode_rv = bpg_pkg_decode_data(chanel->m_incoming_buf, dp_req_data(req), &buf_size, em, chanel->m_debug);
    if (decode_rv == dr_cvt_result_not_enough_output) {
        if (cur_data_size < chanel->m_pkg_max_size) {
            cur_data_size *= 2;
            goto DECODE_PKG;
        }
        else {
            CPE_ERROR(chanel->m_em, "%s: incoming_recv: decode pkg require buf too big, cur_data_size=%d!", conn_net_bpg_chanel_name(chanel), (int)cur_data_size);
            return -1;
        }
    }
    else if (decode_rv != 0) {
        CPE_ERROR(chanel->m_em, "%s: incoming_recv: decode pkg fail, rv=%d!", conn_net_bpg_chanel_name(chanel), (int)decode_rv);
        return -1;
    }
    
    dp_req_set_size(chanel->m_incoming_buf, decode_rv);

    return dp_dispatch_by_string(chanel->m_incoming_dispatch_to, dp_req_mgr(chanel->m_incoming_buf), chanel->m_incoming_buf, em);
}

int conn_net_bpg_chanel_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    conn_net_bpg_chanel_t chanel = ctx;
    size_t cur_data_size = 2048;
    dr_cvt_result_t encode_rv;
    size_t buf_size;

    if (chanel->m_conn_head == NULL) {
        chanel->m_conn_head = conn_net_cli_pkg_create(chanel->m_conn_net_cli);
        if (chanel->m_conn_head == NULL) {
            CPE_ERROR(chanel->m_em, "%s: outgoing_recv: conn_head create fail!", conn_net_bpg_chanel_name(chanel));
            return -1;
        }
    }

    while(cur_data_size < dp_req_size(req)) {
        cur_data_size *= 2;
    }

ENCODE_PKG:
    if (chanel->m_outgoing_buf && dp_req_capacity(chanel->m_outgoing_buf) < cur_data_size) {
        dp_req_free(chanel->m_outgoing_buf);
        chanel->m_outgoing_buf = NULL;
    }

    if (chanel->m_outgoing_buf == NULL) {
        chanel->m_outgoing_buf = dp_req_create(gd_app_dp_mgr(chanel->m_app), cur_data_size);
        if (chanel->m_outgoing_buf == NULL) {
            CPE_ERROR(chanel->m_em, "%s: outgoing_recv: alloc outgoing_buf fail, size=%d!", conn_net_bpg_chanel_name(chanel), (int)cur_data_size);
            return -1;
        }
    }

    buf_size = dp_req_capacity(chanel->m_outgoing_buf);
    encode_rv = bpg_pkg_encode_data(req, dp_req_data(chanel->m_outgoing_buf), &buf_size, em, chanel->m_debug);
    if (encode_rv == dr_cvt_result_not_enough_output) {
        if (cur_data_size < chanel->m_pkg_max_size) {
            cur_data_size *= 2;
            goto ENCODE_PKG;
        }
        else {
            CPE_ERROR(chanel->m_em, "%s: outgoing_recv: encode pkg require buf too big, cur_data_size=%d!", conn_net_bpg_chanel_name(chanel), (int)cur_data_size);
            return -1;
        }
    }
    else if (encode_rv != 0) {
        CPE_ERROR(chanel->m_em, "%s: outgoing_recv: encode pkg fail, rv=%d!", conn_net_bpg_chanel_name(chanel), encode_rv);
        return -1;
    }

    conn_net_cli_pkg_set_sn(chanel->m_conn_head, bpg_pkg_sn(req));
    dp_req_set_parent(conn_net_cli_pkg_to_dp_req(chanel->m_conn_head), chanel->m_outgoing_buf);
    dp_req_set_size(chanel->m_outgoing_buf, buf_size);
    dp_req_set_meta(chanel->m_outgoing_buf, bpg_pkg_manage_basepkg_meta(chanel->m_bpg_pkg_manage));

    return dp_dispatch_by_string(chanel->m_outgoing_dispatch_to, dp_req_mgr(chanel->m_outgoing_buf), chanel->m_outgoing_buf, em);
}
