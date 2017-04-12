#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "gd/app/app_context.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"
#include "mongo_utils.h"

#define MONGO_BUF_APPEND_STR(__data)                    \
    do {                                                \
        uint32_t __len = strlen(__data) + 1;            \
        assert((buf_len - writepos) >= (__len));        \
        memcpy(((char*)buf) + writepos, (__data), (__len)); \
        writepos += (__len);                                \
    } while(0)

#define MONGO_BUF_APPEND_32(__data)                         \
    assert((buf_len - writepos) >= 4);                      \
    MONGO_COPY_HTON32(((char *)buf) + writepos, &(__data));   \
    writepos += 4;

#define MONGO_BUF_APPEND_64(__data)                         \
    assert((buf_len - writepos) >= 8);                      \
    MONGO_COPY_HTON64(((char *)buf) + writepos, &(__data));   \
    writepos += 8;

int mongo_connection_send(mongo_connection_t connection, mongo_pkg_t pkg) {
    mongo_driver_t driver = connection->m_server->m_driver;
    ringbuffer_block_t blk;
    void * buf;
    char ns_buf[65];
    size_t writepos = 0;
    uint32_t reserve = 0;
    uint32_t buf_len = mongo_pkg_calc_len(pkg);

    if (mongo_connection_alloc(&blk, driver, connection, buf_len) != 0) return -1;
    assert(blk);

    ringbuffer_data(driver->m_ringbuf, blk, buf_len, 0, &buf);
    assert(buf);

    MONGO_BUF_APPEND_32(buf_len); /*messageLength*/
    MONGO_BUF_APPEND_32(pkg->m_pro_head.id); /*requestID*/
    MONGO_BUF_APPEND_32(pkg->m_pro_head.response_to); /*responseTo*/
    MONGO_BUF_APPEND_32(pkg->m_pro_head.op); /*opCode*/

    switch(pkg->m_pro_head.op) {
    case mongo_db_op_query: {
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_query.flags);
        snprintf(ns_buf, sizeof(ns_buf), "%s.%s", pkg->m_db, pkg->m_collection);
        MONGO_BUF_APPEND_STR(ns_buf); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_query.number_to_skip);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_query.number_to_return);
        break;
    }
    case mongo_db_op_get_more: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        snprintf(ns_buf, sizeof(ns_buf), "%s.%s", pkg->m_db, pkg->m_collection);
        MONGO_BUF_APPEND_STR(ns_buf); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_get_more.number_to_return);
        MONGO_BUF_APPEND_64(pkg->m_pro_data.m_get_more.cursor_id);
        break;
    }
    case mongo_db_op_insert: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        snprintf(ns_buf, sizeof(ns_buf), "%s.%s", pkg->m_db, pkg->m_collection);
        MONGO_BUF_APPEND_STR(ns_buf); /*ns*/
        break;
    }
    case mongo_db_op_replay: {
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_reply.response_flag);
        MONGO_BUF_APPEND_64(pkg->m_pro_data.m_reply.cursor_id);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_reply.starting_from);
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_reply.number_retuned);
        break;
    }
    case mongo_db_op_msg: {
        break;
    }
    case mongo_db_op_kill_cursors: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_kill_cursor.cursor_count);
        break;
    }
    case mongo_db_op_update: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        snprintf(ns_buf, sizeof(ns_buf), "%s.%s", pkg->m_db, pkg->m_collection);
        MONGO_BUF_APPEND_STR(ns_buf); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_update.flags);
        break;
    }
    case mongo_db_op_delete: {
        MONGO_BUF_APPEND_32(reserve); /*options*/
        snprintf(ns_buf, sizeof(ns_buf), "%s.%s", pkg->m_db, pkg->m_collection);
        MONGO_BUF_APPEND_STR(ns_buf); /*ns*/
        MONGO_BUF_APPEND_32(pkg->m_pro_data.m_delete.flags);
        break;
    }
    default:
        CPE_ERROR(driver->m_em, "%s: send: unknown op %d!", mongo_driver_name(driver), pkg->m_pro_head.op);
        return -1;
    }

    memcpy(((char *)buf) + writepos, mongo_pkg_data(pkg), mongo_pkg_size(pkg));

    mongo_connection_link_node_w(connection, blk);
    connection->m_to_send_pkg_count++;

    mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_wb_update);

    if (driver->m_debug >= 2) {
        CPE_INFO(
            driver->m_em, "%s: server %s.%d: send one pkg:\n%s",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
            mongo_pkg_dump(pkg, &driver->m_dump_buffer, 1));
    }

    return 0;
}
