#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "conn_svr_ops.h"

void conn_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg) {
    conn_svr_t svr = ctx;
    uint32_t cur_time_s = conn_svr_cur_time(svr);
    uint32_t timeout_time = cur_time_s - svr->m_conn_timeout_s;

    while(!TAILQ_EMPTY(&svr->m_conns_check)) {
        conn_svr_conn_t conn = TAILQ_FIRST(&svr->m_conns_check);

        if (conn->m_last_op_time > timeout_time) break;

        CPE_ERROR(
            svr->m_em, "%s: conn(conn_id=%d, fd=%d, user_id="FMT_UINT64_T"): timeout, last-op-time=%d, cur-time=%d!",
            conn_svr_name(svr), conn->m_data.conn_id, conn->m_fd, conn->m_data.user_id, conn->m_last_op_time, cur_time_s);

        conn_svr_conn_free(conn);
    }
}

