#ifndef USF_MONGO_DRIVER_REQUEST_H
#define USF_MONGO_DRIVER_REQUEST_H
#include "bson.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "mongo_driver_types.h"
#include "mongo_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_mongo_pkg;

mongo_pkg_t
mongo_pkg_create(mongo_driver_t agent, size_t capacity);

void mongo_pkg_free(mongo_pkg_t pkg);

mongo_driver_t mongo_pkg_driver(mongo_pkg_t pkg);

dp_req_t mongo_pkg_to_dp_req(mongo_pkg_t pkg);
mongo_pkg_t mongo_pkg_from_dp_req(dp_req_t pkg);

void mongo_pkg_init(mongo_pkg_t pkg);
uint32_t mongo_pkg_size(mongo_pkg_t pkg);
void * mongo_pkg_data(mongo_pkg_t pkg);
int mongo_pkg_set_size(mongo_pkg_t pkg, uint32_t size);
size_t mongo_pkg_capacity(mongo_pkg_t pkg);

/*pkg operations*/
mongo_db_op_t mongo_pkg_op(mongo_pkg_t pkg);
void mongo_pkg_set_op(mongo_pkg_t pkg, mongo_db_op_t op);

void mongo_pkg_doc_it(mongo_doc_it_t doc_it, mongo_pkg_t pkg);
int mongo_pkg_doc_open(mongo_pkg_t pkg);
int mongo_pkg_doc_close(mongo_pkg_t pkg);
int mongo_pkg_doc_count(mongo_pkg_t pkg);
mongo_doc_t mongo_pkg_doc_at(mongo_pkg_t pkg, uint32_t doc_idx);
int mongo_pkg_doc_is_closed(mongo_pkg_t pkg);

int mongo_pkg_doc_append(mongo_pkg_t pkg, LPDRMETA meta, void const * data, size_t capacity);

uint32_t mongo_pkg_id(mongo_pkg_t pkg);
void mongo_pkg_set_id(mongo_pkg_t pkg, uint32_t id);

uint32_t mongo_pkg_response_to(mongo_pkg_t pkg);
void mongo_pkg_set_response_to(mongo_pkg_t pkg, uint32_t id);

const char *mongo_pkg_db(mongo_pkg_t pkg);
int mongo_pkg_set_db(mongo_pkg_t pkg, const char * ns);
const char * mongo_pkg_collection(mongo_pkg_t pkg);
int mongo_pkg_set_collection(mongo_pkg_t pkg, const char * ns);
int mongo_pkg_set_ns(mongo_pkg_t pkg, const char * ns);

const char * mongo_pkg_dump(mongo_pkg_t req, mem_buffer_t buffer, int level);
int mongo_pkg_build_from_cfg(mongo_pkg_t req, cfg_t cfg, error_monitor_t em);

int mongo_iterator_find(bson_iter_t * it, const char * path);

int mongo_pkg_it(bson_iter_t * it, mongo_pkg_t pkg, int doc_idx);
int mongo_pkg_find(bson_iter_t * it, mongo_pkg_t pkg, int doc_idx, const char * path);

int mongo_pkg_validate(mongo_pkg_t pkg, error_monitor_t em);

int mongo_pkg_append_int32(mongo_pkg_t pkg, const char *name, const int32_t i);
int mongo_pkg_append_int64(mongo_pkg_t pkg, const char *name, const int64_t i);
int mongo_pkg_append_double(mongo_pkg_t pkg, const char *name, const double d);
int mongo_pkg_append_string(mongo_pkg_t pkg, const char *name, const char *str);
int mongo_pkg_append_string_n(mongo_pkg_t pkg, const char *name, const char *str, int len);
int mongo_pkg_append_symbol(mongo_pkg_t pkg, const char *name, const char *str);
int mongo_pkg_append_symbol_n(mongo_pkg_t pkg, const char *name, const char *str, int len);
int mongo_pkg_append_code(mongo_pkg_t pkg, const char *name, const char *str);
int mongo_pkg_append_code_n(mongo_pkg_t pkg, const char *name, const char *str, int len);
/* int mongo_pkg_append_code_w_scope(mongo_pkg_t pkg, const char *name, const char *code, const mongo_pkg *scope); */
/* int mongo_pkg_append_code_w_scope_n(mongo_pkg_t pkg, const char *name, const char *code, int size, const mongo_pkg *scope); */
int mongo_pkg_append_binary(mongo_pkg_t pkg, const char *name, char type, const char *str, int len);
int mongo_pkg_append_bool(mongo_pkg_t pkg, const char *name, const int v);
int mongo_pkg_append_null(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_undefined(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_regex(mongo_pkg_t pkg, const char *name, const char *pattern, const char *opts);
int mongo_pkg_append_timestamp(mongo_pkg_t pkg, const char *name, int time, int increment);
int mongo_pkg_append_start_object(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_start_array(mongo_pkg_t pkg, const char *name);
int mongo_pkg_append_finish_object(mongo_pkg_t pkg);
int mongo_pkg_append_finish_array(mongo_pkg_t pkg);
int mongo_pkg_append_object(mongo_pkg_t pkg, const char *name, LPDRMETA meta, void const * data, size_t capacity);
int mongo_pkg_append_object_open(mongo_pkg_t pkg, LPDRMETA meta, void const * data, size_t capacity);    

void mongo_pkg_doc_count_update(mongo_pkg_t pkg);

int32_t mongo_doc_size(mongo_doc_t doc);
void * mongo_doc_data(mongo_doc_t doc);
mongo_doc_t mongo_doc_find_doc(mongo_doc_t doc, const char * path);

#define mongo_pkg_doc_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*mongo req cmd operations*/
void mongo_pkg_cmd_init(mongo_pkg_t pkg);

/*mongo req update operations*/
int32_t mongo_pkg_update_flags(mongo_pkg_t pkg);
void mongo_pkg_update_set_flag(mongo_pkg_t pkg, mongo_pro_flags_update_t flag);
void mongo_pkg_update_unset_flag(mongo_pkg_t pkg, mongo_pro_flags_update_t flag);
    
/*mongo req query operations*/
int32_t mongo_pkg_query_flags(mongo_pkg_t pkg);
void mongo_pkg_query_set_flag(mongo_pkg_t pkg, mongo_pro_flags_query_t flag);
void mongo_pkg_query_unset_flag(mongo_pkg_t pkg, mongo_pro_flags_query_t flag);

int32_t mongo_pkg_query_number_to_skip(mongo_pkg_t pkg);
void mongo_pkg_query_set_number_to_skip(mongo_pkg_t pkg, int32_t number_to_skip);

int32_t mongo_pkg_query_number_to_return(mongo_pkg_t pkg);
void mongo_pkg_query_set_number_to_return(mongo_pkg_t pkg, int32_t number_to_return);

mongo_read_mode_t mongo_pkg_read_mode(mongo_pkg_t pkg);
void mongo_pkg_set_read_mode(mongo_pkg_t pkg, mongo_read_mode_t read_mode);

extern bson_oid_t mongo_driver_oid_zero;
    
#ifdef __cplusplus
}
#endif

#endif
