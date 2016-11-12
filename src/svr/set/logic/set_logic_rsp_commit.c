#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_data.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "set_logic_rsp_ops.h"
#include "protocol/set/logic/set_logic_rsp_carry_info.h"

static dp_req_t set_logic_rsp_commit_build_response(set_logic_rsp_t rsp, logic_context_t op_context, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em);
static dp_req_t set_logic_rsp_commit_build_error_response(set_logic_rsp_t rsp, logic_context_t op_context, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em);

void set_logic_rsp_commit(logic_context_t op_context, void * user_data) {
    set_logic_rsp_t set_logic_rsp;
    set_logic_rsp_manage_t rsp_mgr;
    error_monitor_t em;
    logic_data_t bpg_private_data;
    SET_LOGIC_CARRY_INFO * bpg_private;
    dp_req_t response_buf;

    set_logic_rsp = (set_logic_rsp_t)user_data;
    assert(set_logic_rsp);

    rsp_mgr = set_logic_rsp->m_mgr;
    assert(rsp_mgr);

    em = rsp_mgr->m_em;

    bpg_private_data = logic_context_data_find(op_context, "set_logic_carry_info");
    if (bpg_private_data == NULL) {
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_commit: no set_logic_carry_info in context!",
            set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        set_logic_rsp_manage_free_context(rsp_mgr, op_context);
        return;
    }

    bpg_private = (SET_LOGIC_CARRY_INFO *)logic_data_data(bpg_private_data);
    if (bpg_private->response == 0) {
        if (rsp_mgr->m_debug >= 2) {
            CPE_INFO(
                em, "%s.%s: set_logic_rsp_commit: ignore send response!",
                set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        }
        set_logic_rsp_manage_free_context(rsp_mgr, op_context);
        return;
    }

    if (logic_context_errno(op_context) != 0 && set_svr_svr_info_error_pkg_meta(rsp_mgr->m_svr_type)) {
        goto SEND_ERROR_RESPONSE;
    }

    response_buf = set_logic_rsp_commit_build_response(set_logic_rsp, op_context, bpg_private, em);
    if (response_buf == NULL) {
        goto SEND_ERROR_RESPONSE;
    }

    if (dp_dispatch_by_string(rsp_mgr->m_commit_to, gd_app_dp_mgr(rsp_mgr->m_app), response_buf, em) != 0) {
        CPE_ERROR(em, "%s.%s: set_logic_rsp_commit: dispatch fail!", set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        goto SEND_ERROR_RESPONSE;
    }

    set_logic_rsp_manage_free_context(rsp_mgr, op_context);
    return;

SEND_ERROR_RESPONSE:
    response_buf = set_logic_rsp_commit_build_error_response(set_logic_rsp, op_context, bpg_private, em);
    if (response_buf == NULL) {
        CPE_ERROR(em, "%s.%s: set_logic_rsp_commit: build error response fail!", set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        set_logic_rsp_manage_free_context(rsp_mgr, op_context);
        return;
    }

    if (dp_dispatch_by_string(rsp_mgr->m_commit_to, gd_app_dp_mgr(rsp_mgr->m_app), response_buf, em) != 0) {
        CPE_ERROR(em, "%s.%s: set_logic_rsp_commit: send error response fail!", set_logic_rsp_manage_name(rsp_mgr), set_logic_rsp_name(set_logic_rsp));
        set_logic_rsp_manage_free_context(rsp_mgr, op_context);
        return;
    }

    set_logic_rsp_manage_free_context(rsp_mgr, op_context);
    return;
}

static int set_logic_rsp_commit_build_response_head(
    set_logic_rsp_t rsp, logic_context_t op_context, dp_req_t pkg_body, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em)
{
    dp_req_t response_head;

    response_head = set_pkg_head_check_create(pkg_body);
    if (response_head == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen response buf: response head is NULL!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return -1;
    }

    set_pkg_init(response_head);
    set_pkg_set_sn(response_head, bpg_private->sn);
    set_pkg_set_category(response_head, set_pkg_response);
    set_pkg_set_to_svr(response_head, bpg_private->from_svr_type, bpg_private->from_svr_id);

    return 0;
}

static dp_req_t set_logic_rsp_commit_build_response_body(
    set_logic_rsp_t rsp, logic_context_t op_context, 
    uint32_t cmd, LPDRMETA data_meta, const void * data, size_t data_size, error_monitor_t em)
{
    set_logic_rsp_manage_t mgr;
    size_t total_size;
    size_t data_start_pos;
    dp_req_t response_buf;

    mgr = rsp->m_mgr;

    data_start_pos = dr_entry_data_start_pos(mgr->m_pkg_data_entry, 0);
    total_size = data_size + data_start_pos;

    response_buf = set_logic_rsp_manage_rsp_buf(mgr, total_size);
    if (response_buf == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen response buf: response buf is NULL!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    dp_req_set_meta(response_buf, mgr->m_pkg_meta);
    dp_req_set_size(response_buf, total_size);

    /*处理包头 */
    if (data) {
        memcpy(
            ((char*)dp_req_data(response_buf)) + data_start_pos,
            data, data_size);
    }

    assert(mgr->m_pkg_cmd_entry);
    assert(dr_entry_data_start_pos(mgr->m_pkg_cmd_entry, 0) + sizeof(uint32_t) <= data_start_pos);

    if (dr_entry_set_from_uint32(
            ((char*)dp_req_data(response_buf)) + dr_entry_data_start_pos(mgr->m_pkg_cmd_entry, 0),
            cmd, mgr->m_pkg_cmd_entry, em) != 0)
    {
        CPE_ERROR(
            em, "%s.%s: gen response buf: set cmd fail!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    return response_buf;
}

static int set_logic_rsp_commit_build_response_carry(
    set_logic_rsp_t rsp, logic_context_t op_context, dp_req_t pkg_body, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em)
{
    dp_req_t response_carry;
    LPDRMETA set_carry_data_meta;
    set_svr_svr_info_t from_svr_type;
    logic_data_t carry_data;

    from_svr_type = set_svr_svr_info_find(rsp->m_mgr->m_stub, bpg_private->from_svr_type);
    set_carry_data_meta = from_svr_type ? set_svr_svr_info_carry_meta(from_svr_type) : NULL;

    if (set_carry_data_meta) {
        size_t data_size;

        carry_data = logic_context_data_find(op_context, dr_meta_name(set_carry_data_meta));
        if (carry_data == NULL) {
            CPE_ERROR(
                em, "%s.%s: gen response buf: carry data %s not exist in context!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), dr_meta_name(set_carry_data_meta));
            return -1;
        }

        data_size = dr_meta_calc_data_len(set_carry_data_meta, logic_data_data(carry_data), logic_data_capacity(carry_data));
        if (data_size > (size_t)UCHAR_MAX) {
            CPE_ERROR(
                em, "%s.%s: gen response buf: carry data size %d overflow!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), (int)(data_size));
            return -1;
        }

        response_carry = set_pkg_carry_check_create(pkg_body, data_size + 1);
        if (response_carry == NULL) {
            CPE_ERROR(
                em, "%s.%s: gen response buf: response carry (size=%d) is NULL!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), (int)(data_size + 1));
            return -1;
        }

        if (set_pkg_carry_copy(response_carry, (uint8_t)data_size, logic_data_data(carry_data)) != 0
            || set_pkg_carry_set_size(response_carry, (uint8_t)data_size) != 0)
        {
            CPE_ERROR(
                em, "%s.%s: gen response buf: set pkg data error!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
            return -1;
        }
    }
    else {
        size_t carry_data_size;

        carry_data = logic_context_data_find(op_context, req_type_set_pkg_carry);
        if (carry_data == NULL) {
            CPE_ERROR(
                em, "%s.%s: gen response buf: carry data %s not exist in context!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), req_type_set_pkg_carry);
            return -1;
        }

        carry_data_size = dr_meta_calc_data_len(logic_data_meta(carry_data), logic_data_data(carry_data), logic_data_capacity(carry_data));

        response_carry = set_pkg_carry_check_create(pkg_body, carry_data_size);
        if (response_carry == NULL) {
            CPE_ERROR(
                em, "%s.%s: gen response buf: response carry is NULL!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
            return -1;
        }

        memcpy(dp_req_data(response_carry), logic_data_data(carry_data), carry_data_size);
        dp_req_set_size(response_carry, carry_data_size);
    }

    return 0;
}

static dp_req_t set_logic_rsp_commit_build_response(
    set_logic_rsp_t rsp, logic_context_t op_context, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em)
{
    set_logic_rsp_manage_t mgr;
    LPDRMETA data_meta;
    void * data_buf;
    size_t data_size;
    dp_req_t response_buf;

    mgr = rsp->m_mgr;

    data_meta = set_svr_svr_info_find_data_meta_by_cmd(mgr->m_svr_type, bpg_private->response);
    if (data_meta == NULL) {
        data_buf = NULL;
        data_size = 0;
    }
    else {
        logic_data_t data = logic_context_data_find(op_context, dr_meta_name(data_meta));
        if (data == NULL) {
            CPE_ERROR(
                em, "%s.%s: gen response pkg: can`t find %s from ctx!",
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), dr_meta_name(data_meta));
            return NULL;
        }

        data_buf = logic_data_data(data);
        data_size = dr_meta_calc_data_len(data_meta, logic_data_data(data), logic_data_capacity(data));
    }

    response_buf = 
        set_logic_rsp_commit_build_response_body(
            rsp, op_context, bpg_private->response, data_meta, data_buf, data_size, em);
    if (response_buf == NULL) return NULL;

    if (set_logic_rsp_commit_build_response_head(rsp, op_context, response_buf, bpg_private, em) != 0) return NULL;
    if (set_logic_rsp_commit_build_response_carry(rsp, op_context, response_buf, bpg_private, em) != 0) return NULL;

    return response_buf;
}

static dp_req_t set_logic_rsp_commit_build_error_response(
    set_logic_rsp_t rsp, logic_context_t op_context, SET_LOGIC_CARRY_INFO * bpg_private, error_monitor_t em)
{
    set_logic_rsp_manage_t mgr;
    dp_req_t response_buf;
    char data[128];
    size_t data_size;
    int err;
    LPDRMETA error_pkg_meta;
    LPDRMETAENTRY error_entry;

    mgr = rsp->m_mgr;

    error_pkg_meta = set_svr_svr_info_error_pkg_meta(mgr->m_svr_type);
    if (error_pkg_meta == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen error response buf: no error pkg meta!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    error_entry = set_svr_svr_info_error_pkg_errno_entry(mgr->m_svr_type);
    if (error_pkg_meta == NULL) {
        CPE_ERROR(
            em, "%s.%s: gen error response buf: no error entry!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    data_size = dr_meta_size(error_pkg_meta);
    if (data_size > sizeof(data)) {
        CPE_ERROR(
            em, "%s.%s: gen error response buf: size of %s overflow, (size=%d)!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), dr_meta_name(error_pkg_meta), (int)data_size);
        return NULL;
    }

    err = logic_context_errno(op_context);
    if (err == 0) err = -1;
    if (dr_entry_set_from_int32(((char*)data) + dr_entry_data_start_pos(error_entry, 0), err, error_entry, em) != 0) {
        CPE_ERROR(em, "%s.%s: gen error response buf: set errno fail!", set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp));
        return NULL;
    }

    response_buf = 
        set_logic_rsp_commit_build_response_body(
            rsp, op_context, set_svr_svr_info_error_pkg_cmd(mgr->m_svr_type),
            error_pkg_meta, data, data_size, em);
    if (response_buf == NULL) return NULL;

    if (set_logic_rsp_commit_build_response_head(rsp, op_context, response_buf, bpg_private, em) != 0) return NULL;
    if (set_logic_rsp_commit_build_response_carry(rsp, op_context, response_buf, bpg_private, em) != 0) return NULL;

    return response_buf;
}

void set_logic_rsp_manage_free_context(set_logic_rsp_manage_t mgr, logic_context_t op_context) {
    if (mgr->m_ctx_fini) {
        mgr->m_ctx_fini(op_context, mgr->m_ctx_ctx);
    }
    logic_context_free(op_context);
}
