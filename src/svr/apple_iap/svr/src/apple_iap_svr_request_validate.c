#include <assert.h> 
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/base64.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_task.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "apple_iap_svr_ops.h"

static void apple_iap_svr_request_validate_commit(net_trans_task_t task, void * ctx);
static int apple_iap_svr_request_send_request(apple_iap_svr_t svr, net_trans_task_t task);

static void apple_iap_svr_request_validate_send_error_response(
    apple_iap_svr_t svr, const char * receipt, int error,
    uint32_t sn, uint16_t from_svr_type, uint16_t from_svr_id);

void apple_iap_svr_request_validate(apple_iap_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head) {
    net_trans_task_t task;
    SVR_APPLE_IAP_REQ_VALIDATE * req;
    struct apple_iap_task_data * task_data;

    req = &((SVR_APPLE_IAP_PKG*)dp_req_data(pkg_body))->data.svr_apple_iap_req_validate;

    task = net_trans_task_create(svr->m_trans_group, sizeof(struct apple_iap_task_data));
    if (task == NULL) {
        CPE_ERROR(svr->m_em, "%s: validate: create task fail!", apple_iap_svr_name(svr));

        apple_iap_svr_request_validate_send_error_response(
            svr, req->receipt, -1,
            set_pkg_sn(pkg_head), set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head));

        return;
    }

    task_data = net_trans_task_data(task);

    cpe_str_dup(task_data->m_receipt, sizeof(task_data->m_receipt), req->receipt);
    task_data->m_sn = set_pkg_sn(pkg_head);
    task_data->m_from_svr_type = set_pkg_from_svr_type(pkg_head);
    task_data->m_from_svr_id = set_pkg_from_svr_id(pkg_head);
    task_data->m_is_sandbox = svr->m_is_sandbox;

    net_trans_task_set_commit_op(task, apple_iap_svr_request_validate_commit, svr, NULL);

    if (apple_iap_svr_request_send_request(svr, task) != 0) {
        net_trans_task_free(task);
        return;
    }        

    return;
}

static void apple_iap_svr_request_validate_commit(net_trans_task_t task, void * ctx) {
    apple_iap_svr_t svr = ctx;
    struct apple_iap_task_data * task_data = net_trans_task_data(task);
    SVR_APPLE_IAP_RES_VALIDATE res;
    const char * response;
    mem_buffer_t response_buf;

    if (net_trans_task_result(task) != net_trans_result_ok) {
        int err = -1;

        CPE_ERROR(
            svr->m_em, "%s: validate: task not ok, result is %s!",
            apple_iap_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)));

        if (net_trans_task_result(task) != net_trans_result_timeout) {
            err = SVR_APPLE_IAP_ERRNO_VALIDATE_TIMEOUT;
        }

        apple_iap_svr_request_validate_send_error_response(
            svr, task_data->m_receipt, -1,
            task_data->m_sn, task_data->m_from_svr_type, task_data->m_from_svr_id);

        return;
    }

    response_buf = net_trans_task_buffer(task);
    assert(response_buf);

    mem_buffer_append_char(response_buf, 0);

    response = mem_buffer_make_continuous(response_buf, 0);
    if (response == NULL) {
        CPE_ERROR(svr->m_em, "%s: validate: task not ok, response is NULL!", apple_iap_svr_name(svr));

        apple_iap_svr_request_validate_send_error_response(
            svr, task_data->m_receipt, -1,
            task_data->m_sn, task_data->m_from_svr_type, task_data->m_from_svr_id);

        return;
    }

    bzero(&res, sizeof(res));
    res.status = 0xcccccccc;
    if (dr_json_read(&res, sizeof(res), response, svr->m_meta_res_validate, svr->m_em) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: validate: parse response fail%s!",
            apple_iap_svr_name(svr), response);

        apple_iap_svr_request_validate_send_error_response(
            svr, task_data->m_receipt, -1,
            task_data->m_sn, task_data->m_from_svr_type, task_data->m_from_svr_id);

        return;
    }

    if ((uint32_t)res.status == 0xcccccccc) {
        CPE_ERROR(
            svr->m_em, "%s: validate: read status fail%s!",
            apple_iap_svr_name(svr), response);

        apple_iap_svr_request_validate_send_error_response(
            svr, task_data->m_receipt, -1,
            task_data->m_sn, task_data->m_from_svr_type, task_data->m_from_svr_id);

        return;
    }

    if (res.status == 21007) {
        task_data->m_is_sandbox =  1;
        if (apple_iap_svr_request_send_request(svr, task) == 0) {
            return;
        }
    }

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: validate: receive response%s!", apple_iap_svr_name(svr), response);
    }

    if (set_svr_stub_send_response_data(
            svr->m_stub, task_data->m_from_svr_type, task_data->m_from_svr_id, task_data->m_sn,
            &res, sizeof(res), svr->m_meta_res_validate, NULL, 0)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: validate: send response error!", apple_iap_svr_name(svr));
        return;
    }
}

static void apple_iap_svr_request_validate_send_error_response(
    apple_iap_svr_t svr, const char * receipt, int error,
    uint32_t sn, uint16_t from_svr_type, uint16_t from_svr_id)
{
    SVR_APPLE_IAP_RES_ERROR res;
    res.error = error;

    if (set_svr_stub_send_response_data(
            svr->m_stub, from_svr_type, from_svr_id, sn,
            &res, sizeof(res), svr->m_meta_res_error, NULL, 0)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: validate: send error response error!", apple_iap_svr_name(svr));
        return;
    }
}

static int apple_iap_svr_request_send_request(apple_iap_svr_t svr, net_trans_task_t task) {
    struct apple_iap_task_data * task_data = net_trans_task_data(task);
    char request_buf[SVR_APPLE_IAP_RECEIPT_MAX * 2];
    struct write_stream_mem request_stream = CPE_WRITE_STREAM_MEM_INITIALIZER(request_buf, sizeof(request_buf));
    struct read_stream_mem receipt_stream;

    /*构造请求 */
    read_stream_mem_init(&receipt_stream, task_data->m_receipt, strlen(task_data->m_receipt));

    stream_printf((write_stream_t)&request_stream, "{\"receipt-data\":\"");
    cpe_base64_encode((write_stream_t)&request_stream, (read_stream_t)&receipt_stream);
    stream_printf((write_stream_t)&request_stream, "\"}");
    stream_putc((write_stream_t)&request_stream, 0);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: validate: send request %s!", apple_iap_svr_name(svr), request_buf);
    }

    /*发送请求 */
    if (net_trans_task_set_post_to(
            task, 
            task_data->m_is_sandbox
            ? "https://sandbox.itunes.apple.com/verifyReceipt"
            : "https://buy.itunes.apple.com/verifyReceipt"
            ,
            request_buf,
            strlen(request_buf))
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: validate: post %s fail!", apple_iap_svr_name(svr), request_buf);
        return -1;
    }

    if (net_trans_task_start(task) != 0) {
        CPE_ERROR(svr->m_em, "%s: validate: start request fail!", apple_iap_svr_name(svr));
        return -1;
    }
    
    return 0;
}
