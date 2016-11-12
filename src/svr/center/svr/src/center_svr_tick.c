#include <assert.h>
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr.h"
#include "center_svr_set_proxy.h"
#include "center_svr_ins_proxy.h"

static void center_svr_conn_op_notify_leave(center_svr_t svr, center_svr_set_proxy_t set) {
    SVR_CENTER_PKG * notify_pkg;
    struct cpe_hash_it notify_set_it;
    center_svr_set_proxy_t notify_set;
    
    notify_pkg = center_svr_get_res_pkg_buff(svr, NULL, sizeof(SVR_CENTER_PKG));
    if (notify_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: leave: alloc notify buf fail!", center_svr_name(svr));
        return;
    }
    
    notify_pkg->cmd = SVR_CENTER_CMD_NTF_LEAVE;
    notify_pkg->data.svr_center_ntf_leave.set_id = set->m_set->id;

    cpe_hash_it_init(&notify_set_it, &svr->m_set_proxies);
    while((notify_set = cpe_hash_it_next(&notify_set_it))) {
        if (notify_set->m_conn == NULL) continue;

        center_svr_conn_send(notify_set->m_conn, notify_pkg, sizeof(SVR_CENTER_PKG));
    }
}

static void center_svr_check_set_timeout(center_svr_t svr) {
    struct cpe_hash_it set_it;
    center_svr_set_proxy_t set, next;
    uint32_t cur_time = center_svr_cur_time(svr);

    cpe_hash_it_init(&set_it, &svr->m_set_proxies);
    for(set = cpe_hash_it_next(&set_it); set; set = next) {
        next = cpe_hash_it_next(&set_it);
        if (set->m_conn) continue;
        if (set->m_offline_time + svr->m_set_offline_timeout > cur_time) continue;

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: set %d: offline timeout, offline-time=%d, timeout=%d, cur-time=%d, auto remove!",
                center_svr_name(svr), set->m_set->id, set->m_offline_time, svr->m_set_offline_timeout, cur_time);
        }

        center_svr_conn_op_notify_leave(svr, set);

        while(!TAILQ_EMPTY(&set->m_ins_proxies)) {
            center_svr_ins_proxy_t ins = TAILQ_FIRST(&set->m_ins_proxies);
            void * ins_data = ins->m_data;
            center_svr_ins_proxy_free(ins);
            aom_obj_free(svr->m_client_data_mgr, ins_data);
        }

        center_svr_set_proxy_free(set);
    }
}

ptr_int_t center_svr_tick(void * ctx, ptr_int_t arg, float delta_s) {
    center_svr_t svr = ctx;

    if (svr->m_set_offline_timeout > 0) center_svr_check_set_timeout(svr);

    return 0;
}

