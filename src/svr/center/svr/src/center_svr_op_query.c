#include <assert.h> 
#include "center_svr_conn.h"
#include "center_svr_type.h"
#include "center_svr_ins_proxy.h"

void center_svr_conn_op_query_by_type(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    SVR_CENTER_PKG * res;
    size_t res_len;
    SVR_CENTER_RES_QUERY * res_records;
    struct cpe_hash_it set_ins_it;
    center_svr_ins_proxy_t set_ins;

    res_len = sizeof(SVR_CENTER_PKG) + cpe_hash_table_count(&svr->m_ins_proxies) * sizeof(SVR_CENTER_SVR_INFO);
    res = center_svr_get_res_pkg_buff(svr, pkg, res_len);
    if (res == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: query by type: alloc res buf fail!",
            center_svr_name(svr), conn->m_fd);
        return;
    }

    res->cmd = SVR_CENTER_CMD_RES_QUERY;

    res_records = &res->data.svr_center_res_query;

    res_records->count = 0;
    cpe_hash_it_init(&set_ins_it, &svr->m_ins_proxies);
    while((set_ins = cpe_hash_it_next(&set_ins_it))) {
        SVR_CENTER_SVR_INFO * res_record;

        res_record = &res_records->data[res_records->count++];
        res_record->svr.svr_type = set_ins->m_data->svr_type;
        res_record->svr.svr_id = set_ins->m_data->svr_id;
        res_record->set = set_ins->m_data->set;
    }

    center_svr_conn_send(conn, res, res_len);
}
