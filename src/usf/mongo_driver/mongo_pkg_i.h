#ifndef USF_MONGO_DRIVER_PKG_I_H
#define USF_MONGO_DRIVER_PKG_I_H
#include "bson.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_driver_i.h"

struct mongo_pro_header {
    int32_t id;
    int32_t response_to;
    int32_t op;
};

union mongo_pro_data {
    struct mongo_pro_data_update m_update;
    struct mongo_pro_data_insert m_insert;
    struct mongo_pro_data_query m_query;
    struct mongo_pro_data_get_more m_get_more;
    struct mongo_pro_data_delete m_delete;
    struct mongo_pro_data_kill_cursors m_kill_cursor;
    struct mongo_pro_data_reply m_reply;
};

struct mongo_pkg {
    mongo_driver_t m_driver;
    dp_req_t m_dp_req;
    int32_t m_stack[32];
    int32_t m_stackPos;
    uint32_t m_reserve;
    mongo_read_mode_t m_read_mode;
    char m_db[32];
    char m_collection[32];
    int32_t m_doc_count;
    int32_t m_cur_doc_start;
    int32_t m_cur_doc_pos;
    struct mongo_pro_header m_pro_head;
    union mongo_pro_data m_pro_data;
};

enum mongo_pkg_recv_result {
    mongo_pkg_recv_error = -1
    , mongo_pkg_recv_ok = 0
    , mongo_pkg_recv_not_enough_data = 1
};

#define MONGO_EMPTY_DOCUMENT_SIZE (5)

uint32_t mongo_pkg_calc_len(mongo_pkg_t pkg);

#endif
