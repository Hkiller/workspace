#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "account_svr_module.h"
#include "account_svr_conn_info.h"
#include "protocol/svr/conn/svr_conn_data.h"
#include "protocol/svr/conn/svr_conn_pro.h"

int account_svr_conn_get_conn_info(account_svr_t svr, logic_context_t ctx, uint32_t * conn_id, uint64_t * account_id) {
    logic_data_t conn_data;
    CONN_SVR_CONN_INFO const * conn_info;

    conn_data = logic_context_data_find(ctx, "conn_svr_conn_info");
    if (conn_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: get_conn_info: conn_svr_conn_info not exist!", account_svr_name(svr));
        return -1;
    }
    conn_info = logic_data_data(conn_data);

    if (conn_id) *conn_id = conn_info->conn_id;
    if (account_id) *account_id = conn_info->user_id;

    return 0;
}

int account_svr_is_conn_svr(account_svr_t svr, uint16_t svr_type_id) {
    set_svr_svr_info_t from_svr_info;

    from_svr_info = set_svr_svr_info_find(svr->m_stub, svr_type_id);

    return
        (from_svr_info
         && set_svr_svr_info_pkg_meta(from_svr_info)
         && strcmp(dr_meta_name(set_svr_svr_info_pkg_meta(from_svr_info)), "svr_conn_pkg") == 0)
        ? 1 : 0;
}

int account_svr_conn_bind_account(
    account_svr_t svr, logic_context_t ctx,
    uint16_t conn_svr_id, uint16_t conn_svr_type, uint64_t account_id, uint16_t account_state,
    uint8_t device_category, uint8_t device_cap, const char * chanel)
{
    dp_req_t pkg_buf;
    SVR_CONN_PKG * pkg;
    uint32_t conn_id;

    if (account_svr_conn_get_conn_info(svr, ctx, &conn_id, NULL) != 0) {
        APP_CTX_ERROR(svr->m_app, "%s: bind_account: get conn id fail!", account_svr_name(svr));
        return -1;
    }

    pkg_buf = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_CONN_PKG));
    if (pkg_buf == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: bind_account: get pkg buf fail!", account_svr_name(svr));
        return -1;
    }
    pkg = dp_req_data(pkg_buf);

    pkg->cmd = SVR_CONN_CMD_REQ_BIND_USER;
    pkg->data.svr_conn_req_bind_user.data.conn_id = conn_id;
    pkg->data.svr_conn_req_bind_user.data.user_id = account_id;
    pkg->data.svr_conn_req_bind_user.data.user_state = account_state;
    pkg->data.svr_conn_req_bind_user.data.device_cap = device_cap;
    pkg->data.svr_conn_req_bind_user.data.device_category = device_category;
    cpe_str_dup(
        pkg->data.svr_conn_req_bind_user.data.chanel,
        sizeof(pkg->data.svr_conn_req_bind_user.data.chanel),
        chanel);

    if (set_svr_stub_send_req_pkg(svr->m_stub, conn_svr_type, conn_svr_id, 0, pkg_buf, NULL, 0) != 0) {
        APP_CTX_ERROR(svr->m_app, "%s: bind_account: send bind client req fail!", account_svr_name(svr));
        return -1;
    }

    return 0;
}
