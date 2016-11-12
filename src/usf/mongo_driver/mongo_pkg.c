#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "mongo_pkg_i.h"
#include "mongo_utils.h"

mongo_pkg_t
mongo_pkg_create(mongo_driver_t driver, size_t capacity) {
    dp_req_t dp_req;
    mongo_pkg_t pkg;

    dp_req = dp_req_create(
        gd_app_dp_mgr(driver->m_app),
        sizeof(struct mongo_pkg) + capacity);
    if (dp_req == NULL) return NULL;

    dp_req_set_type(dp_req, req_type_mongo_pkg);

    pkg = (mongo_pkg_t)dp_req_data(dp_req);

    pkg->m_driver = driver;
    pkg->m_dp_req = dp_req;

    mongo_pkg_init(pkg);

    return pkg;
}

void mongo_pkg_free(mongo_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

dp_req_t mongo_pkg_to_dp_req(mongo_pkg_t req) {
    return req->m_dp_req;
}

mongo_pkg_t mongo_pkg_from_dp_req(dp_req_t req) {
    if (!dp_req_is_type(req, req_type_mongo_pkg)) return NULL;
    return (mongo_pkg_t)dp_req_data(req);
}

mongo_driver_t mongo_pkg_driver(mongo_pkg_t req) {
    return req->m_driver;
}

void mongo_pkg_init(mongo_pkg_t pkg) {
    pkg->m_read_mode = mongo_read_unknown;
    pkg->m_stackPos = 0;
    pkg->m_db[0] = 0;
    pkg->m_collection[0] = 0;
    pkg->m_doc_count = 0;
    pkg->m_cur_doc_start = -1;
    pkg->m_cur_doc_pos = -1;
    bzero(&pkg->m_pro_head, sizeof(pkg->m_pro_head));
    bzero(&pkg->m_pro_data, sizeof(pkg->m_pro_data));
    mongo_pkg_set_size(pkg, 0);
}

mongo_db_op_t mongo_pkg_op(mongo_pkg_t pkg) {
    return pkg->m_pro_head.op;
}

void mongo_pkg_set_op(mongo_pkg_t pkg, mongo_db_op_t op) {
    pkg->m_pro_head.op = op;
    bzero(&pkg->m_pro_data, sizeof(pkg->m_pro_data));

    switch(op) {
    case mongo_db_op_query:
        pkg->m_pro_data.m_query.number_to_return = 10;
        break;
    case mongo_db_op_get_more:
        pkg->m_pro_data.m_get_more.number_to_return = 10;
        break;
    default:
        break;
    }
}

uint32_t mongo_pkg_id(mongo_pkg_t pkg) {
    return pkg->m_pro_head.id;
}

void mongo_pkg_set_id(mongo_pkg_t pkg, uint32_t id) {
    pkg->m_pro_head.id = id;
}

uint32_t mongo_pkg_response_to(mongo_pkg_t pkg) {
    return pkg->m_pro_head.response_to;
}

void mongo_pkg_set_response_to(mongo_pkg_t pkg, uint32_t id) {
    pkg->m_pro_head.response_to = id;
}

const char * mongo_pkg_db(mongo_pkg_t pkg) {
    return pkg->m_db;
}

int mongo_pkg_set_db(mongo_pkg_t pkg, const char * db) {
    size_t len = strlen(db);
    if (len + 1 > sizeof(pkg->m_db)) {
        CPE_ERROR(pkg->m_driver->m_em, "mongo_pkg: set db %s: db name len overflow!", db);
        return -1;
    }

    memcpy(pkg->m_db, db, len + 1);
    return 0;
}

const char * mongo_pkg_collection(mongo_pkg_t pkg) {
    return pkg->m_collection;
}

int mongo_pkg_set_collection(mongo_pkg_t pkg, const char * collection) {
    size_t len = strlen(collection);
    if (len + 1 > sizeof(pkg->m_collection)) {
        CPE_ERROR(pkg->m_driver->m_em, "mongo_pkg: set collection %s: collection name len overflow!", collection);
        return -1;
    }

    memcpy(pkg->m_collection, collection, len + 1);
    return 0;
}

int mongo_pkg_set_ns(mongo_pkg_t pkg, const char * ns) {
    const char * sep;
    size_t len;

    sep = strchr(ns, '.');
    if (sep == NULL) {
        CPE_ERROR(pkg->m_driver->m_em, "mongo_pkg: set ns %s: ns format error!", ns);
        return -1;
    }

    len = sep - ns;
    if (len + 1 > sizeof(pkg->m_db)) {
        CPE_ERROR(pkg->m_driver->m_em, "mongo_pkg: set ns %s: db name len overflow!", ns);
        return -1;
    }

    memcpy(pkg->m_db, ns, len);
    pkg->m_db[len] = 0;

    len = strlen(sep + 1);
    if (len + 1 > sizeof(pkg->m_collection)) {
        CPE_ERROR(pkg->m_driver->m_em, "mongo_pkg: set ns %s: collection name len overflow!", ns);
        return -1;
    }

    memcpy(pkg->m_collection, sep + 1, len + 1);

    return 0;
}

mongo_read_mode_t mongo_pkg_read_mode(mongo_pkg_t pkg) {
    return pkg->m_read_mode;
}

void mongo_pkg_set_read_mode(mongo_pkg_t pkg, mongo_read_mode_t read_mode) {
    pkg->m_read_mode = read_mode;
}

void * mongo_pkg_data(mongo_pkg_t pkg) {
    return (pkg + 1);
}

int mongo_pkg_doc_open(mongo_pkg_t pkg) {
    uint32_t new_size;

    if (pkg->m_cur_doc_start >= 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_start: pkg is already started",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    assert(pkg->m_stackPos == 0);

    new_size = mongo_pkg_size(pkg) + 4;
    if (mongo_pkg_set_size(pkg, new_size) != 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_start: pkg size overflow, new_size=%d, capacity=%d",
            mongo_driver_name(pkg->m_driver), new_size, (int)mongo_pkg_capacity(pkg));
        return -1;
    }

    ++pkg->m_doc_count;

    pkg->m_cur_doc_start = new_size - 4;
    pkg->m_cur_doc_pos =  new_size;

    return 0;
}

int mongo_pkg_doc_close(mongo_pkg_t pkg) {
    uint32_t new_size;
    uint32_t doc_size;
    char * buf;

    if (pkg->m_cur_doc_start < 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_close: document is already closed",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    if (pkg->m_stackPos != 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_close: document data is not closed",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    new_size = mongo_pkg_size(pkg) + 1;
    if (mongo_pkg_set_size(pkg, new_size) != 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_close: pkg size overflow, new_size=%d, capacity=%d",
            mongo_driver_name(pkg->m_driver), new_size, (int)mongo_pkg_capacity(pkg));
        return -1;
    }

    buf = (char *)(pkg + 1);
    doc_size = pkg->m_cur_doc_pos - pkg->m_cur_doc_start + 1;
    MONGO_COPY_HTON32(buf + pkg->m_cur_doc_start, &doc_size);
    buf[new_size - 1] = 0;

    mongo_pkg_set_size(pkg, new_size);

    pkg->m_cur_doc_pos = -1;
    pkg->m_cur_doc_start = -1;

    return 0;
}

int mongo_pkg_set_size(mongo_pkg_t pkg, uint32_t size) {
    if (dp_req_set_size(pkg->m_dp_req, sizeof(struct mongo_pkg) + size) != 0) {
        return -1;
    }

    return 0;
}

uint32_t mongo_pkg_size(mongo_pkg_t pkg) {
    return dp_req_size(pkg->m_dp_req) - sizeof(struct mongo_pkg);
}

size_t mongo_pkg_capacity(mongo_pkg_t pkg) {
    return dp_req_capacity(pkg->m_dp_req) - sizeof(struct mongo_pkg);
}

mongo_doc_t mongo_pkg_doc_at(mongo_pkg_t pkg, uint32_t doc_idx) {
    struct mongo_doc_it doc_it;
    void * doc;

    if (doc_idx >= pkg->m_doc_count) return NULL;

    if (doc_idx == 0) {
        return (mongo_doc_t)mongo_pkg_data(pkg);
    }

    mongo_pkg_doc_it(&doc_it, pkg);

    while(doc_idx > 0 && (doc = mongo_pkg_doc_it_next(&doc_it))) {
        --doc_idx;
    }

    return doc_idx == 0 ? doc : NULL;
}

int mongo_pkg_it(bson_iter_t * it, mongo_pkg_t pkg, int doc_idx) {
    mongo_doc_t doc;
    bson_t data;
    
    doc = mongo_pkg_doc_at(pkg, doc_idx);
    if (doc == NULL) return -1;

    bson_init_static(&data, (const uint8_t *)doc, (uint32_t)mongo_doc_size(doc));
    bson_iter_init(it, &data);
    
    return 0;
}

static int mongo_iterator_find_l1(bson_iter_t * it, const char * name, const char * name_end) {
    size_t strlen = name_end - name;
    while(bson_iter_next(it)) {
        const char * key = bson_iter_key(it);
        if (memcmp(key, name, strlen) == 0 && key[strlen] == 0)
            return 0;
    }

    return -1;
}

int mongo_iterator_find(bson_iter_t * it, const char * path) {
    const char * end = path + strlen(path);
    const char * name = path;
    const char * name_end;
    do {
        name_end = strchr(name, '.');
        if (name_end == NULL) name_end = end;

        if (mongo_iterator_find_l1(it, name, name_end) != 0) return -1;
        name = name_end + 1;
    } while(name_end != end);

    return 0;
}

int mongo_pkg_find(bson_iter_t * it, mongo_pkg_t pkg, int doc_idx, const char * path) {
    if (mongo_pkg_it(it, pkg, doc_idx) != 0) return -1;
    return mongo_iterator_find(it, path);
}

void mongo_pkg_doc_count_update(mongo_pkg_t pkg) {
    uint32_t doc_start = 0;
    uint32_t doc_size;
    uint32_t total_size = mongo_pkg_size(pkg);
    char * buf = (char*)(pkg + 1);

    pkg->m_doc_count = 0;

    while(doc_start + 4 < total_size) {
        ++pkg->m_doc_count;

        MONGO_COPY_HTON32(&doc_size, buf + doc_start);
        doc_start += doc_size;
    }
}

int mongo_pkg_doc_count(mongo_pkg_t pkg) {
    return pkg->m_doc_count;
}

static void mongo_pkg_data_dump_i(write_stream_t stream, const void * data, uint32_t size, int depth) {
    bson_t i_data;
    bson_iter_t i;
    const char *key;
    char oidhex[25];
    bson_t scope;
    
    bson_init_static(&i_data, (const uint8_t *)data, size);
    bson_iter_init(&i, &i_data);

    while(bson_iter_next(&i)) {
        const bson_value_t * v;

        v = bson_iter_value(&i);
        if (v == NULL) break;
        
        key = bson_iter_key( &i);

        stream_putc_count(stream,  ' ', depth << 2);

        switch (v->value_type) {
        case BSON_TYPE_DOUBLE:
            stream_printf(stream,  "%s : %d(%s)\t%f" , key, v->value_type, "BSON_DOUBLE", bson_iter_double( &i ));
            break;
        case BSON_TYPE_UTF8:
            stream_printf(stream,  "%s : %d(%s)\t[%s]" , key, v->value_type, "BSON_UTF8", bson_iter_utf8(&i, NULL));
            break;
        case BSON_TYPE_SYMBOL:
            stream_printf(stream,  "%s : %d(%s)\t[%s]" , key, v->value_type, "BSON_SYMBOL", bson_iter_symbol(&i, NULL));
            break;
        case BSON_TYPE_OID:
            bson_oid_to_string( bson_iter_oid( &i ), oidhex);
            stream_printf(stream,  "%s : %d(%s)\t[%s]" , key, v->value_type, "BSON_OID", oidhex);
            break;
        case BSON_TYPE_BOOL:
            stream_printf(stream,  "%s : %d(%s)\t%s" , key, v->value_type, "BSON_BOOL", bson_iter_bool( &i ) ? "true" : "false");
            break;
        case BSON_TYPE_BINARY:
            stream_printf(stream,  "%s : %d(%s)", key, v->value_type, "BSON_BINDATA");
            break;
        case BSON_TYPE_UNDEFINED:
            stream_printf(stream,  "%s : %d(%s)", key, v->value_type, "BSON_UNDEFINED");
            break;
        case BSON_TYPE_NULL:
            stream_printf(stream,  "%s : %d(%s)", key, v->value_type, "BSON_NULL");
            break;
        case BSON_TYPE_REGEX:
            stream_printf(stream,  "%s : %d(%s)\t: /%s/", key, v->value_type, "BSON_REGEX", bson_iter_regex(&i, NULL));
            break;
        case BSON_TYPE_CODE:
            stream_printf(stream,  "%s : %d(%s)\t: {%s}", key, v->value_type, "BSON_CODE", bson_iter_code(&i, NULL));
            break;
        case BSON_TYPE_CODEWSCOPE:
            stream_printf(stream,  "%s : %d(%s)\t: {%s}", key, v->value_type, "BSON_CODE_W_SCOPE", bson_iter_code(&i, NULL));
            bson_init( &scope);
            bson_iter_codewscope(&i, NULL, NULL, NULL);
            break;
        case BSON_TYPE_INT32:
            stream_printf(stream,  "%s : %d(%s)\t%d" , key, v->value_type, "BSON_INT", bson_iter_int32(&i));
            break;
        case BSON_TYPE_INT64:
            stream_printf(stream,  "%s : %d(%s)\t%I64d", key, v->value_type, "BSON_LONG", bson_iter_int64(&i));
            break;
        case BSON_TYPE_DATE_TIME:
            stream_printf(stream,  "%s : %d(%s)\t%I64d", key, v->value_type, "BSON_LONG", bson_iter_int64(&i));
            break;
        case BSON_TYPE_TIMESTAMP:
            stream_printf(stream,  "%s : %d(%s)\t%I64d", key, v->value_type, "BSON_TIMESTAMP", v->value.v_int64);
            break;
        case BSON_TYPE_DOCUMENT:
            stream_printf(stream,  "%s : %d(%s)\n", key, v->value_type, "BSON_OBJECT");
            mongo_pkg_data_dump_i(stream, v->value.v_doc.data, v->value.v_doc.data_len, depth + 1);
            break;
        case BSON_TYPE_ARRAY:
            stream_printf(stream,  "%s : %d(%s)\n", key, v->value_type, "BSON_ARRAY");
            mongo_pkg_data_dump_i(stream, v->value.v_doc.data, v->value.v_doc.data_len, depth + 1);
            break;
        default:
            stream_printf(stream, "%s : %d(%s)" , key, v->value_type, "???");
        }
        stream_printf(stream, "\n");
    }
}

const char * mongo_pkg_dump(mongo_pkg_t req, mem_buffer_t buffer, int level) {
    struct write_stream_buffer stream;
    struct mongo_doc_it doc_it;
    mongo_doc_t doc;
    int i = 0;

    mem_buffer_clear_data(buffer);

    write_stream_buffer_init(&stream, buffer);

    stream_putc_count((write_stream_t)&stream,  ' ', level << 2);
    stream_printf((write_stream_t)&stream, "*********** head **********\n", i);
    stream_putc_count((write_stream_t)&stream,  ' ', (level + 1) << 2);
    stream_printf(
        (write_stream_t)&stream, "id=%d, response_to=%d, ",
        req->m_pro_head.id, req->m_pro_head.response_to);

    switch(req->m_pro_head.op) {
    case mongo_db_op_query:
        stream_printf(
            (write_stream_t)&stream, "op=query, ns=%s.%s, flags=%d, number_to_skip=%d, number_to_return=%d",
            req->m_db, req->m_collection, req->m_pro_data.m_query.flags, req->m_pro_data.m_query.number_to_skip, req->m_pro_data.m_query.number_to_return);
        break;
    case mongo_db_op_get_more:
        stream_printf(
            (write_stream_t)&stream, "op=get more, ns=%s.%s, number_to_return=%d, cursor_id=%llu",
            req->m_db, req->m_collection, req->m_pro_data.m_get_more.number_to_return, req->m_pro_data.m_get_more.cursor_id);
        break;
    case mongo_db_op_insert:
        stream_printf((write_stream_t)&stream, "op=insert, ns=%s.%s", req->m_db, req->m_collection);
        break;
    case mongo_db_op_replay:
        stream_printf((write_stream_t)&stream, "op=replay, response_flag=(");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_cursor_not_found) stream_printf((write_stream_t)&stream, " cursor_not_found");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_query_fail) stream_printf((write_stream_t)&stream, " query_fail");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_shared_config_state) stream_printf((write_stream_t)&stream, " shared_config_state");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_await_capable) stream_printf((write_stream_t)&stream, " await_capable");
        stream_printf(
            (write_stream_t)&stream, "), cursor_id=%llu, starting_from=%d, number_retuned=%d",
            req->m_pro_data.m_reply.cursor_id, req->m_pro_data.m_reply.starting_from, req->m_pro_data.m_reply.number_retuned);
        break;
    case mongo_db_op_msg:
        stream_printf((write_stream_t)&stream, "op=msg");
        break;
    case mongo_db_op_kill_cursors:
        stream_printf((write_stream_t)&stream, "op=kill cursors, cursor_count=%d", req->m_pro_data.m_kill_cursor.cursor_count);
        break;
    case mongo_db_op_update:
        stream_printf((write_stream_t)&stream, "op=update, ns=%s.%s, flags=%d", req->m_db, req->m_collection, req->m_pro_data.m_update.flags);
        break;
    case mongo_db_op_delete:
        stream_printf((write_stream_t)&stream, "op=delete, ns=%s.%s, flags=%d", req->m_db, req->m_collection, req->m_pro_data.m_delete.flags);
        break;
    default:
        stream_printf((write_stream_t)&stream, "op=%d, unknown!", req->m_pro_head.op);
        break;
    }
    stream_printf((write_stream_t)&stream, "\n");

    i = 0;
    mongo_pkg_doc_it(&doc_it, req);
    while((doc = mongo_pkg_doc_it_next(&doc_it))) {

        stream_putc_count((write_stream_t)&stream,  ' ', level << 2);
        stream_printf((write_stream_t)&stream, "*********** doc %d **********\n", i);

        mongo_pkg_data_dump_i((write_stream_t)&stream, mongo_doc_data(doc), (uint32_t)mongo_doc_size(doc), level + 1);

        stream_printf((write_stream_t)&stream, "\n");

        ++i;
    }

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

int mongo_pkg_validate(mongo_pkg_t pkg, error_monitor_t em) {
    if (pkg->m_cur_doc_start != -1) {
        CPE_ERROR(
            em, "%s: pkg_validate: pkg document not closed",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    return 0;
}

struct mongo_pkg_doc_it_data {
    mongo_pkg_t m_pkg;
    int32_t m_pos;
};

static mongo_doc_t mongo_pkg_doc_it_do_next(struct mongo_doc_it * it) {
    struct mongo_pkg_doc_it_data * data = (struct mongo_pkg_doc_it_data *)&it->m_data;
    uint32_t total_size;
    uint32_t doc_size;
    char * buf;
    void * r;

    if (data->m_pkg == NULL) return NULL;

    total_size =  mongo_pkg_size(data->m_pkg);
    buf = (char*)(data->m_pkg + 1);

    if (data->m_pos < 0 || data->m_pos + 4 > total_size) return NULL;
    MONGO_COPY_HTON32(&doc_size, buf + data->m_pos);

    if (doc_size < MONGO_EMPTY_DOCUMENT_SIZE) return NULL;

    if (data->m_pos + doc_size > total_size) return NULL;

    r = buf + data->m_pos;

    data->m_pos += doc_size;

    return r;
}

void mongo_pkg_doc_it(mongo_doc_it_t it, mongo_pkg_t pkg) {
    struct mongo_pkg_doc_it_data * data = (struct mongo_pkg_doc_it_data *)&it->m_data;

    assert(sizeof(*data) <= sizeof(it->m_data));
    data->m_pkg = pkg;
    data->m_pos = 0;
    it->next = mongo_pkg_doc_it_do_next;
}

int32_t mongo_doc_size(mongo_doc_t doc) {
    int32_t doc_size;
    MONGO_COPY_NTOH32(&doc_size, doc);
    return doc_size;
}

void * mongo_doc_data(mongo_doc_t doc) {
    return (void*)doc;
}

mongo_doc_t mongo_doc_find_doc(mongo_doc_t doc, const char * path) {
    bson_iter_t it;
    bson_t data;
    uint8_t t;
    bson_value_t * v;
        
    bson_init_static(&data, (const uint8_t *)doc, (uint32_t)mongo_doc_size(doc));
    bson_iter_init(&it, &data);

    if (mongo_iterator_find(&it, path) != 0) return NULL;

    v = bson_iter_value(&it);

    if (v->value_type != BSON_TYPE_DOCUMENT) return NULL;

    return (mongo_doc_t)v->value.v_doc.data;
}

void mongo_pkg_cmd_init(mongo_pkg_t pkg) {
    mongo_pkg_init(pkg);
    mongo_pkg_set_op(pkg, mongo_db_op_query);
    mongo_pkg_set_collection(pkg, "$cmd");
    mongo_pkg_query_set_number_to_return(pkg, 1);
}

int32_t mongo_pkg_update_flags(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_update);
    return pkg->m_pro_data.m_update.flags;
}

void mongo_pkg_update_set_flag(mongo_pkg_t pkg, mongo_pro_flags_update_t flag) {
    assert(pkg->m_pro_head.op == mongo_db_op_update);
    pkg->m_pro_data.m_update.flags |= (uint32_t)flag;
}

void mongo_pkg_update_unset_flag(mongo_pkg_t pkg, mongo_pro_flags_update_t flag) {
    assert(pkg->m_pro_head.op == mongo_db_op_update);
    pkg->m_pro_data.m_update.flags &= ~ ((uint32_t)flag);
}

int32_t mongo_pkg_query_flags(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    return pkg->m_pro_data.m_query.flags;
}

void mongo_pkg_query_set_flag(mongo_pkg_t pkg, mongo_pro_flags_query_t flag) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.flags |= (uint32_t)flag;
}

void mongo_pkg_query_unset_flag(mongo_pkg_t pkg, mongo_pro_flags_query_t flag) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.flags &= ~ ((uint32_t)flag);
}

int32_t mongo_pkg_query_number_to_skip(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    return pkg->m_pro_data.m_query.number_to_skip;
}

void mongo_pkg_query_set_number_to_skip(mongo_pkg_t pkg, int32_t number_to_skip) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.number_to_skip = number_to_skip;
}

int32_t mongo_pkg_query_number_to_return(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    return pkg->m_pro_data.m_query.number_to_return;
}

void mongo_pkg_query_set_number_to_return(mongo_pkg_t pkg, int32_t number_to_return) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.number_to_return = number_to_return;
}

uint32_t mongo_pkg_calc_len(mongo_pkg_t pkg) {
    size_t ns_len = strlen(pkg->m_db) + 1 + strlen(pkg->m_collection) + 1;

    uint32_t len = 
        16 /* header */
        + mongo_pkg_size(pkg) /*document*/
        + 0 /*fields*/
        ;

    switch(pkg->m_pro_head.op) {
    case mongo_db_op_query: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t) + sizeof(int32_t);
        break;
    }
    case mongo_db_op_get_more: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t) + sizeof(int64_t);

        break;
    }
    case mongo_db_op_insert: {
        len += sizeof(int32_t) + ns_len;
        break;
    }
    case mongo_db_op_replay: {
        len += sizeof(int32_t) + sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t);
        break;
    }
    case mongo_db_op_msg: {
        break;
    }
    case mongo_db_op_kill_cursors: {
        len += sizeof(int32_t) + sizeof(int32_t);
        break;
    }
    case mongo_db_op_update: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t);
        break;
    }
    case mongo_db_op_delete: {
        len += sizeof(int32_t) + ns_len + sizeof(int32_t);
        break;
    }
    }

    return len;
}

const char * req_type_mongo_pkg = "mongo_pkg";
