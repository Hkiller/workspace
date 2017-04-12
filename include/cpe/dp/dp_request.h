#ifndef CPE_DP_REQUEST_H
#define CPE_DP_REQUEST_H
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "dp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dp_req_t dp_req_create(dp_mgr_t mgr, size_t capacity);
void dp_req_free(dp_req_t req);

dp_mgr_t dp_req_mgr(dp_req_t req);

void dp_req_set_parent(dp_req_t child, dp_req_t parent);
void dp_req_add_to_parent(dp_req_t child, dp_req_t parent);
int dp_req_manage_by_parent(dp_req_t req);

dp_req_t dp_req_parent(dp_req_t req);
dp_req_t dp_req_parent_find(dp_req_t req, const char * typeName);
dp_req_t dp_req_brother_find(dp_req_t req, const char * typeName);

void dp_req_childs(dp_req_t req, dp_req_it_t it);
dp_req_t dp_req_child_find(dp_req_t req, const char * typeName);
dp_req_t dp_req_child_first(dp_req_t req);
void dp_req_child_clear(dp_req_t req);

void dp_req_set_buf(dp_req_t req, void * buf, size_t capacity);

void dp_req_set_type(dp_req_t req, const char * type);
const char * dp_req_type(dp_req_t req);
int dp_req_is_type(dp_req_t req, const char * type);

LPDRMETA dp_req_meta(dp_req_t req);
void dp_req_set_meta(dp_req_t req, LPDRMETA meta);

void * dp_req_data(dp_req_t req);
size_t dp_req_capacity(dp_req_t req);
size_t dp_req_size(dp_req_t req);
int dp_req_set_size(dp_req_t req, size_t size);

void dp_req_data_clear(dp_req_t req);

void dp_req_set_dumper(dp_req_t req, dp_req_dump_fun_t dumper);
const char * dp_req_dump(dp_req_t req, mem_buffer_t buffer);
void dp_req_print(dp_req_t req, write_stream_t s, int ident);

#define dp_req_next(it) (it)->next((it))

#ifdef __cplusplus
}
#endif

#endif


