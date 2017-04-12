#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "protocol/svr/dblog/svr_dblog_pro.h"
#include "dblog_svr_i.h"
#include "dblog_svr_meta.h"
#include "dblog_svr_db_ops.h"

static void dblog_svr_process_log(dblog_svr_t svr, char * input, size_t input_size) {
    uint16_t svr_type;
    uint16_t svr_id;
    uint16_t input_used_size = 0;
    uint64_t log_time;
    uint16_t meta_id;
    dblog_svr_meta_t svr_meta;
    int rv;
    
    if (input_size < 14) {
        CPE_ERROR(svr->m_em, "%s: log: input_size %d not enough!", dblog_svr_name(svr), (int)input_size);
        return;
    }
    
    CPE_COPY_NTOH16(&svr_type, input + input_used_size);
    input_used_size += 2;

    CPE_COPY_NTOH16(&svr_id, input + input_used_size);
    input_used_size += 2;

    CPE_COPY_NTOH64(&log_time, input + input_used_size);
    input_used_size += 8;
        
    CPE_COPY_NTOH16(&meta_id, input + input_used_size);
    input_used_size += 2;

    svr_meta = dblog_svr_meta_find(svr, svr_type, meta_id);
    if (svr_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: log: from %d-%d meta %d: meta not exist!", dblog_svr_name(svr), svr_type, svr_id, meta_id);
        return;
    }

    bzero(svr->m_decode_buf, sizeof(svr->m_decode_buf));
    rv = dr_pbuf_read(
        svr->m_decode_buf, sizeof(svr->m_decode_buf),
        input + input_used_size, input_size - input_used_size, svr_meta->m_meta, svr->m_em);
    if (rv < 0) {
        CPE_ERROR(
            svr->m_em, "%s: log: from %d-%d meta %d(%s): decode fail rv=%d, input-size=%d, data-size=%d!",
            dblog_svr_name(svr), svr_type, svr_id, meta_id, dr_meta_name(svr_meta->m_meta), rv, (int)input_size, (int)(input_size - input_used_size));
        return;
    }

    if (dblog_svr_db_insert(svr, svr_id, log_time, svr->m_decode_buf, rv, svr_meta) != 0) {
        return;
    }
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: log: from %d-%d meta %d(%s): input-size=%d, input-data-size=%d, decode-size=%d: %s!\n",
            dblog_svr_name(svr), svr_type, svr_id, meta_id, dr_meta_name(svr_meta->m_meta),
            (int)input_size, (int)(input_size - input_used_size), (int)rv,
            dr_json_dump_inline(&svr->m_dump_buffer, svr->m_decode_buf, rv, svr_meta->m_meta));
    }
}

void dblog_svr_net_cb(EV_P_ ev_io *w, int revents) {
    dblog_svr_t svr = w->data;
    socklen_t bytes;
    uint8_t cmd;
    
    bytes = recvfrom(svr->m_fd, svr->m_input_buf, sizeof(svr->m_input_buf), 0/*flags*/, NULL, 0);
    if (bytes <= 0) {
        CPE_ERROR(svr->m_em, "%s: read data fail, size=%d, error=%d (%s)", dblog_svr_name(svr), (int)bytes, errno, strerror(errno));
        return;
    }

    cmd = svr->m_input_buf[0];
    
    switch(cmd) {
    case SVR_DBLOG_CMD_REQ_LOG:
        dblog_svr_process_log(svr, svr->m_input_buf + 1, bytes - 1);
        break;
    default:
        CPE_ERROR(svr->m_em, "%s: unknown cmd %d", dblog_svr_name(svr), cmd);
        return;
    }
}
