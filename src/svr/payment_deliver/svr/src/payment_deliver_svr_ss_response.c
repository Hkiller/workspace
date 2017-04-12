#include <assert.h>
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "payment_deliver_svr_request.h"
#include "payment_deliver_svr_connection.h"
#include "payment_deliver_svr_adapter.h"
#include "payment_deliver_svr_adapter_type.h"

int payment_deliver_response_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    payment_deliver_svr_t svr = ctx;
    dp_req_t pkg_head;
    uint32_t sn;
    payment_deliver_request_t request;
    payment_deliver_connection_t connection;
    payment_deliver_adapter_t adapter;
    uint32_t r_cmd;
    LPDRMETA r_meta;
    void * r_data;
    size_t r_data_size;
    int32_t svr_error;
    
    pkg_head = set_pkg_head_find(req);
    if (pkg_head == NULL) {
        CPE_ERROR(svr->m_em, "%s: process request: no pkg head!", payment_deliver_svr_name(svr));
        return -1;
    }

    sn = set_pkg_sn(pkg_head);
    if (sn == 0) {
        CPE_ERROR(svr->m_em, "%s: response: response no sn, skip!", payment_deliver_svr_name(svr));
        return -1;
    }

    request = payment_deliver_request_find(svr, sn);
    if (request == NULL) {
        CPE_ERROR(svr->m_em, "%s: response: request %d not exist!", payment_deliver_svr_name(svr), sn);
        return -1;
    }

    connection = request->m_connection;

    if (request->m_state != payment_deliver_request_runing) {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: state is not runing, skip response", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        return -1;
    }

    if (request->m_adapter == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: adapter not exist", 
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    adapter = request->m_adapter;

    if (set_svr_stub_read_data(
            svr->m_stub, svr->m_payment_svr, req,
            &r_cmd, &r_meta, &r_data, &r_data_size)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: read response data fail",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (r_cmd == SVR_PAYMENT_CMD_RES_NOTIFY) {
        svr_error = 0;
    }
    else if (r_cmd == SVR_PAYMENT_CMD_RES_ERROR) {
        if (r_data == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: response: request %d at connection %d: error response no data",
                payment_deliver_svr_name(svr), request->m_id, connection->m_id);
            payment_deliver_request_set_error(request, 500, "Internal Server Error");
            return -1;
        }

        svr_error = ((SVR_PAYMENT_RES_ERROR *)r_data)->error;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: response: request %d at connection %d: receive unknown response %d",
            payment_deliver_svr_name(svr), request->m_id, connection->m_id, r_cmd);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
    
    adapter->m_type->m_on_response(svr, request, svr_error);

    assert(request->m_state == payment_deliver_request_complete);

    return 0;
}
