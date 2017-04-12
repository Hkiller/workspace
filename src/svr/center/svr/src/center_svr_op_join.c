#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_conn.h"
#include "center_svr_ins_proxy.h"
#include "center_svr_set_proxy.h"
#include "center_svr_type.h"

static int8_t center_svr_conn_op_join_sync_data(center_svr_t svr, center_svr_conn_t conn, SVR_CENTER_REQ_JOIN const * req);
static void center_svr_conn_op_notify_join(center_svr_t svr, center_svr_set_proxy_t set);

void center_svr_conn_op_join(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    SVR_CENTER_REQ_JOIN * req = &pkg->data.svr_center_req_join;
    SVR_CENTER_PKG * res;
    int8_t join_rv = 0;

    res = center_svr_get_res_pkg_buff(svr, pkg, sizeof(SVR_CENTER_PKG));
    if (res == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: join: alloc res buf fail!",
            center_svr_name(svr), conn->m_fd);
        join_rv = SVR_CENTER_ERROR_INTERNAL;
        goto SEND_RESPONSE;
    }

    join_rv = center_svr_conn_op_join_sync_data(svr, conn, req);

SEND_RESPONSE:
    res->cmd = SVR_CENTER_CMD_RES_JOIN;
    res->data.svr_center_res_join.result = join_rv;

    center_svr_conn_send(conn, res, sizeof(*res));
}

static int8_t center_svr_conn_op_join_sync_data(center_svr_t svr, center_svr_conn_t conn, SVR_CENTER_REQ_JOIN const * req) {
    uint16_t i;
    center_svr_set_proxy_t set;
    center_svr_ins_proxy_t ins, ins_next;
    uint8_t changed = 0;
    
    set = center_svr_set_proxy_find(svr, req->set.id);
    if (set == NULL) {
        set = center_svr_set_proxy_create(svr, &req->set);
        if (set == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: join: create set %d fail!",
                center_svr_name(svr), conn->m_fd, req->set.id);
            return SVR_CENTER_ERROR_INTERNAL;
        }
        changed = 1;
    }
    else {
        if (!center_svr_set_proxy_is_match(set, &req->set)) {
            center_svr_set_proxy_update(set, &req->set);
            changed = 1;
        }
    }

    if (set->m_conn != conn) {
        center_svr_set_proxy_set_conn(set, conn);
    }

    for(ins = TAILQ_FIRST(&set->m_ins_proxies); ins; ins = ins_next) {
        uint8_t exist = 0;
        ins_next = TAILQ_NEXT(ins, m_next_for_set);
        
        for(i = 0; i < req->svr_count; ++i) {
            SVR_CENTER_SVR_ID const * record = req->svrs + i;
            if (record->svr_type == ins->m_data->svr_type && record->svr_id == ins->m_data->svr_id) {
                exist = 1;
                break;
            }
        }

        if (!exist) {
            void * ins_data = ins->m_data;
            center_svr_ins_proxy_free(ins);
            aom_obj_free(svr->m_client_data_mgr, ins_data);
            changed = 1;
        }
    }

    /**/
    for(i = 0; i < req->svr_count; ++i) {
        SVR_CENTER_SVR_ID const * runing_svr;

        runing_svr = &req->svrs[i];
        
        ins = center_svr_ins_proxy_find(svr, runing_svr->svr_type, runing_svr->svr_id);
        if (ins == NULL) {
            SVR_CENTER_CLI_RECORD * record;
            center_svr_type_t svr_type;
            changed = 1;

            svr_type = center_svr_type_find(svr, runing_svr->svr_type);
            if (svr_type == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: join: svr_type of %d not exist!",
                    center_svr_name(svr), conn->m_fd, runing_svr->svr_type);
                return SVR_CENTER_ERROR_SVR_TYPE_NOT_EXIST;
            }

            record = aom_obj_alloc(svr->m_client_data_mgr);
            if (record == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: join: alloc SVR_CENTER_CLI_RECORD fail!",
                    center_svr_name(svr), conn->m_fd);
                return SVR_CENTER_ERROR_INTERNAL;
            }

            record->svr_type = runing_svr->svr_type;
            record->svr_id = runing_svr->svr_id;
            record->set = req->set;
            record->online_time = 0;

            ins = center_svr_ins_proxy_create(svr, svr_type, set, record);
            if (ins == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: join: create cli data fail!",
                    center_svr_name(svr), conn->m_fd);
                aom_obj_free(svr->m_client_data_mgr, record);
                return SVR_CENTER_ERROR_INTERNAL;
            }
        }
        else {
            if (ins->m_set != set) {
            }
        }
    }

    if (changed) center_svr_conn_op_notify_join(svr, set);
    
    return 0;
}

static void center_svr_conn_op_notify_join(center_svr_t svr, center_svr_set_proxy_t set) {
    SVR_CENTER_PKG * notify_pkg;
    SVR_CENTER_NTF_JOIN * ntf_join;
    struct cpe_hash_it notify_set_it;
    center_svr_set_proxy_t notify_set;
    center_svr_ins_proxy_t ins;
    size_t pkg_capacity = sizeof(SVR_CENTER_PKG) + set->m_ins_count * sizeof(SVR_CENTER_SVR_INFO);
    
    notify_pkg = center_svr_get_res_pkg_buff(svr, NULL, pkg_capacity);
    if (notify_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: join: alloc notify buf fail!", center_svr_name(svr));
        return;
    }
    
    notify_pkg->cmd = SVR_CENTER_CMD_NTF_JOIN;
    ntf_join = &notify_pkg->data.svr_center_ntf_join;
    ntf_join->set_id = set->m_set->id;
    ntf_join->count = 0;

    TAILQ_FOREACH(ins, &set->m_ins_proxies, m_next_for_set) {
        SVR_CENTER_SVR_INFO * svr_info = &ntf_join->data[ntf_join->count++];
        svr_info->set = ins->m_data->set;
        svr_info->svr.svr_id = ins->m_data->svr_id;
        svr_info->svr.svr_type = ins->m_data->svr_type;
    }

    cpe_hash_it_init(&notify_set_it, &svr->m_set_proxies);
    while((notify_set = cpe_hash_it_next(&notify_set_it))) {
        if (notify_set == set) continue;
        if (notify_set->m_conn == NULL) continue;

        center_svr_conn_send(notify_set->m_conn, notify_pkg, pkg_capacity);
    }
}
