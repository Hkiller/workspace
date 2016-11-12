#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/stream_buffer.h"
#include "gd/app/app_context.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"
#include "mongo_utils.h"

static int mongo_connection_process_data(mongo_driver_t driver, mongo_connection_t connection);

void mongo_connection_rw_cb(EV_P_ ev_io *w, int revents) {
    
    mongo_connection_t connection;
    mongo_driver_t driver;
    char * buffer;

    connection = w->data;
    driver = connection->m_server->m_driver;
    CPE_ERROR(driver->m_em, "***************HolleWorld nice to meet you************");

    if (revents & EV_READ) {
        CPE_ERROR(driver->m_em, "***************HolleWorld nice to meet you in if************");
        ringbuffer_block_t blk;
        if (mongo_connection_alloc(&blk, driver, connection, driver->m_read_block_size) != 0) return;
        assert(blk);

        buffer = NULL;
        ringbuffer_block_data(driver->m_ringbuf, blk, 0, (void **)&buffer);
        assert(buffer);
CPE_ERROR(driver->m_em, "***************HolleWorld nice to meet you again************");
        for(;;) {
            CPE_ERROR(driver->m_em, "***************HolleWorld nice to meet you in for************");
            int bytes = cpe_recv(connection->m_fd, buffer, driver->m_read_block_size, 0);
            if (bytes > 0) {
                if (driver->m_debug >= 2) {
                    CPE_INFO(
                        driver->m_em, "%s: server %s.%d: recv %d bytes data!",
                        mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, bytes);
                }

                ringbuffer_shrink(driver->m_ringbuf, blk, bytes);
                mongo_connection_link_node_r(connection, blk);
                break;
            }
            else if (bytes == 0) {
                blk = ringbuffer_yield(driver->m_ringbuf, blk, driver->m_read_block_size);
                assert(blk == NULL);
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: free for recv return 0!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
                mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
                return;
            }
            else {
                assert(bytes == -1);

                switch(errno) {
                case EWOULDBLOCK:
                    blk = ringbuffer_yield(driver->m_ringbuf, blk, driver->m_read_block_size);
                    assert(blk == NULL);
                    break;
                case EINTR:
                    continue;
                default:
                    blk = ringbuffer_yield(driver->m_ringbuf, blk, driver->m_read_block_size);
                    assert(blk == NULL);
                    CPE_ERROR(
                        driver->m_em, "%s: server %s.%d: free for recv error, errno=%d (%s)!",
                        mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
                    mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
                    return;
                }
            }
        }
CPE_ERROR(driver->m_em, "***************HolleWorld bye************");
        if (mongo_connection_process_data(driver, connection) != 0) return;
    }

    if (revents & EV_WRITE) {
        while(connection->m_wb) {
            void * data;
            int block_size;
            int bytes;

            block_size = ringbuffer_block_data(driver->m_ringbuf, connection->m_wb, 0, &data);
            assert(block_size > 0);
            assert(data);

            if (connection->m_sending_pkg_op == 0) {
                assert(block_size >= 16);
                MONGO_COPY_NTOH32(&connection->m_sending_pkg_op, ((const char *)data) + 12);

                assert(connection->m_sending_pkg_op == mongo_db_op_replay
                       || connection->m_sending_pkg_op == mongo_db_op_msg
                       || connection->m_sending_pkg_op == mongo_db_op_update
                       || connection->m_sending_pkg_op == mongo_db_op_insert
                       || connection->m_sending_pkg_op == mongo_db_op_query
                       || connection->m_sending_pkg_op == mongo_db_op_get_more
                       || connection->m_sending_pkg_op == mongo_db_op_delete
                       || connection->m_sending_pkg_op == mongo_db_op_kill_cursors
                    );
            }
            
            bytes = cpe_send(connection->m_fd, data, block_size, 0);
            if (bytes > 0) {
                if (bytes == block_size) {
                    assert(connection->m_to_send_pkg_count > 0);
                    connection->m_to_send_pkg_count--;

                    assert(connection->m_sending_pkg_op != 0);
                    if (connection->m_sending_pkg_op == mongo_db_op_query) {
                        mongo_connection_runing_pkg_push(connection);
                    }
                    connection->m_sending_pkg_op = 0;
                }
                
                if (driver->m_debug >= 2) {
                    CPE_INFO(
                        driver->m_em, "%s: server %s.%d: send %d bytes data%s!",
                        mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, bytes,
                        bytes == block_size ? "(one pkg)" : "");
                }

                connection->m_wb = ringbuffer_yield(driver->m_ringbuf, connection->m_wb, bytes);
                if (bytes < block_size) break;
            }
            else if (bytes == 0) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: free for send return 0!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
                mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
                return;
            }
            else {
                int err = cpe_sock_errno();
                assert(bytes == -1);

                if (err == EWOULDBLOCK) break;
                if (err == EINTR) continue;

                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: free for send error, errno=%d (%s)!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, err, cpe_sock_errstr(err));
                mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
                return;
            }
        }
    }

    if (driver->m_debug >= 3) {
        CPE_INFO(
            driver->m_em, "%s: server %s.%d: dump buff info: %s",
            mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
            ringbuffer_dump(&driver->m_dump_buffer, driver->m_ringbuf));
    }

    mongo_connection_start_watch(connection);
}

static void * mongo_connection_merge_rb(mongo_driver_t driver, mongo_connection_t server) {
    ringbuffer_block_t new_blk;
    void * buf;
    int length = ringbuffer_block_total_len(driver->m_ringbuf, server->m_rb);
    
    if (mongo_connection_alloc(&new_blk, driver, server, length) != 0) return NULL;
    assert(new_blk);

    buf = ringbuffer_copy(driver->m_ringbuf, server->m_rb, 0, new_blk);
    assert(buf);

    ringbuffer_free(driver->m_ringbuf, server->m_rb);
    server->m_rb = new_blk;

    return buf;
}

#define MONGO_BUF_READ_32(__value, __fieldname)                     \
    if (read_pos + 4 > buf_len) {                                   \
        CPE_ERROR(                                                  \
            driver->m_em, "%s: server %s.%d: read "__fieldname" "   \
            "not enouth buf, read_pos=%d, buf_len=%d!",             \
            mongo_driver_name(driver),                              \
            connection->m_server->m_host,                             \
            connection->m_server->m_port,                           \
            read_pos, buf_len);                                     \
        mongo_connection_fsm_apply_evt(                             \
            connection, mongo_connection_fsm_evt_disconnected);     \
        return -1;                                                  \
    }                                                               \
    MONGO_COPY_NTOH32(&(__value), ((const char *)buf) + read_pos);  \
    read_pos += 4

#define MONGO_BUF_READ_64(__value, __fieldname)                         \
    if (read_pos + 8 > buf_len) {                                       \
        CPE_ERROR(                                                      \
            driver->m_em, "%s: server %s.%d: read "__fieldname" "       \
            "not enouth buf, read_pos=%d, buf_len=%d!",                 \
            mongo_driver_name(driver),                                  \
            connection->m_server->m_host,                                 \
            connection->m_server->m_port,                               \
            read_pos, buf_len);                                         \
        mongo_connection_fsm_apply_evt(                                 \
            connection, mongo_connection_fsm_evt_disconnected);         \
        return -1;                                                      \
    }                                                                   \
    MONGO_COPY_NTOH64(&(__value), ((const char *)buf) + read_pos);      \
    read_pos += 8

#define MONGO_BUF_READ_STR(__value, __fieldname)                        \
    do {                                                                \
        uint32_t __len = strlen((const char *)buf + read_pos);          \
        if (read_pos + __len + 1 > buf_len) {                           \
            CPE_ERROR(                                                  \
                driver->m_em, "%s: server %s.%d: read "__fieldname" "   \
                "not enouth buf, read_pos=%d, buf_len=%d, str_len=%d!", \
                mongo_driver_name(driver),                              \
                connection->m_server->m_host,                             \
                connection->m_server->m_port,                           \
                read_pos, buf_len, __len);                              \
            mongo_connection_fsm_apply_evt(                             \
                connection, mongo_connection_fsm_evt_disconnected);     \
            return -1;                                                  \
        }                                                               \
        if (__len + 1 > sizeof(__value)) __len = sizeof(__value) - 1;   \
        memcpy(__value, ((const char *)buf) + read_pos, __len);         \
        __value[__len] = 0;                                             \
        read_pos += __len + 1;                                          \
    } while(0)


static int mongo_connection_process_data(mongo_driver_t driver, mongo_connection_t connection) {
    CPE_ERROR(driver->m_em, "***************HolleWorld************");
    void * buf;
    uint32_t buf_len;
    uint32_t read_pos;
    int received_data_len;
    uint32_t total_len;
    uint32_t reserve;
    char ns_buf[65];
    mongo_pkg_t pkg = NULL;

    while(connection->m_rb) {
        received_data_len = ringbuffer_data(driver->m_ringbuf, connection->m_rb, sizeof(uint32_t), 0, &buf);
        if (received_data_len < sizeof(uint32_t)) return 0; /*缓存数据不够读取包长度 */

        /*数据主够读取包的大小，但是头块太小，无法保存数据头，提前合并一次数据 */
        if (buf == NULL) buf = mongo_connection_merge_rb(driver, connection);
        if (buf == NULL) return -1;

        read_pos = 0;

        buf_len = received_data_len;
        MONGO_BUF_READ_32(total_len, "total_len");

        received_data_len = ringbuffer_data(driver->m_ringbuf, connection->m_rb, total_len, 0, &buf);

        if (total_len > (uint32_t)received_data_len) return 0; /*数据包不完整 */
        
        /*确保获取完整的数据包 */
        ringbuffer_data(driver->m_ringbuf, connection->m_rb, total_len, 0, &buf);
        /*完整的数据包不在一个块内 */
        if (buf == NULL) buf = mongo_connection_merge_rb(driver, connection);
        if (buf == NULL) return -1;

        if (pkg == NULL) {
            pkg = mongo_driver_pkg_buf(driver);
            if (pkg == NULL) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: on read: get pkg buf fail!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port);
                mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_stop);
                return -1;
            }
        }

        mongo_pkg_init(pkg);

        buf_len = total_len;
        MONGO_BUF_READ_32(pkg->m_pro_head.id, "id");
        MONGO_BUF_READ_32(pkg->m_pro_head.response_to, "response_to");
        MONGO_BUF_READ_32(pkg->m_pro_head.op, "op");

        switch(pkg->m_pro_head.op) {
        case mongo_db_op_query: {
            MONGO_BUF_READ_32(pkg->m_pro_data.m_query.flags, "query.flags");
            MONGO_BUF_READ_STR(ns_buf, "query.ns"); /*ns*/
            mongo_pkg_set_ns(pkg, ns_buf);
            MONGO_BUF_READ_32(pkg->m_pro_data.m_query.number_to_skip, "query.number_to_skip");
            MONGO_BUF_READ_32(pkg->m_pro_data.m_query.number_to_return, "query.number_to_return");
            break;
        }
        case mongo_db_op_get_more: {
            MONGO_BUF_READ_32(reserve, "get_more.reserve"); /*options*/
            MONGO_BUF_READ_STR(ns_buf, "get_more.ns"); /*ns*/
            mongo_pkg_set_ns(pkg, ns_buf);
            MONGO_BUF_READ_32(pkg->m_pro_data.m_get_more.number_to_return, "get_more.number_to_return");
            MONGO_BUF_READ_64(pkg->m_pro_data.m_get_more.cursor_id, "get_more.cursor_id");
            break;
        }
        case mongo_db_op_insert: {
            MONGO_BUF_READ_32(reserve, "insert.reserve"); /*options*/
            MONGO_BUF_READ_STR(ns_buf, "insert.ns"); /*ns*/
            mongo_pkg_set_ns(pkg, ns_buf);
            break;
        }
        case mongo_db_op_replay: {
            MONGO_BUF_READ_32(pkg->m_pro_data.m_reply.response_flag, "reply.response_flag");
            MONGO_BUF_READ_64(pkg->m_pro_data.m_reply.cursor_id, "reply.cursor_id");
            MONGO_BUF_READ_32(pkg->m_pro_data.m_reply.starting_from, "reply.starting_from");
            MONGO_BUF_READ_32(pkg->m_pro_data.m_reply.number_retuned, "reply.number_retuned");
            break;
        }
        case mongo_db_op_msg: {
            break;
        }
        case mongo_db_op_kill_cursors: {
            MONGO_BUF_READ_32(reserve, "kill_cursors.reserve"); /*options*/
            MONGO_BUF_READ_32(pkg->m_pro_data.m_kill_cursor.cursor_count, "kill_cursor.cursor_count");
            break;
        }
        case mongo_db_op_update: {
            MONGO_BUF_READ_32(reserve, "update.reserve"); /*options*/
            MONGO_BUF_READ_STR(ns_buf, "update.ns"); /*ns*/
            mongo_pkg_set_ns(pkg, ns_buf);
            MONGO_BUF_READ_32(pkg->m_pro_data.m_update.flags, "update.flags");
            break;
        }
        case mongo_db_op_delete: {
            MONGO_BUF_READ_32(reserve, "delete.reserve"); /*options*/
            MONGO_BUF_READ_STR(ns_buf, "delete.ns"); /*ns*/
            mongo_pkg_set_ns(pkg, ns_buf);
            MONGO_BUF_READ_32(pkg->m_pro_data.m_delete.flags, "delete.flags");
            break;
        }
        default:
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: op(%d) is not reply, skip!",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, pkg->m_pro_head.op);
            mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
            return -1;
        }

        assert(total_len >= read_pos);

        if (total_len > read_pos) {
            uint32_t doc_size = total_len - read_pos;
            if (doc_size < MONGO_EMPTY_DOCUMENT_SIZE) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: doc len is too small for empty doc, doc-len=%d!",
                    mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
                    total_len - read_pos);
                mongo_connection_fsm_apply_evt(connection, mongo_connection_fsm_evt_disconnected);
                return -1;
            }

            memcpy(mongo_pkg_data(pkg), (const char *)buf + read_pos, doc_size);
            read_pos += doc_size;
            mongo_pkg_set_size(pkg, doc_size);
            mongo_pkg_doc_count_update(pkg);
        }
CPE_ERROR(driver->m_em, "***************HolleWorld meet debug, debug=%d************", driver->m_debug);
        if (driver->m_debug >= 2) {
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: recv one pkg: doc-size=%d, doc-count=%d\n%s\n",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port,
                (int)mongo_pkg_size(pkg), (int)mongo_pkg_doc_count(pkg),
                mongo_pkg_dump(pkg, &driver->m_dump_buffer, 1));
        }

        if (pkg->m_pro_head.op == mongo_db_op_replay) {
            mongo_connection_runing_pkg_pop(connection);
        }
        
        mongo_connection_fsm_apply_recv_pkg(connection, pkg);
        if(connection->m_rb == NULL) return -1;

        /*移除已经获取的数据 */
        connection->m_rb = ringbuffer_yield(driver->m_ringbuf, connection->m_rb, total_len);

        if (driver->m_debug >= 3) {
            CPE_INFO(
                driver->m_em, "%s: server %s.%d: yield %d bytes",
                mongo_driver_name(driver), connection->m_server->m_host, connection->m_server->m_port, total_len);
        }
    }

    return 0;
}
