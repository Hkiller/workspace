#ifndef SVR_SET_SHARE_PKG_H
#define SVR_SET_SHARE_PKG_H
#include "set_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_set_pkg_head;
extern const char * req_type_set_pkg_carry;

dp_req_t set_pkg_head_find(dp_req_t body);
dp_req_t set_pkg_head_check_create(dp_req_t body);
dp_req_t set_pkg_head_create(dp_mgr_t dp_mgr);

dp_req_t set_pkg_carry_find(dp_req_t body);
dp_req_t set_pkg_carry_check_create(dp_req_t body, size_t capacity);
dp_req_t set_pkg_carry_create(dp_mgr_t dp_mgr, size_t capacity);

void set_pkg_init(dp_req_t head);
int set_pkg_init_response(dp_req_t body, dp_req_t request_body);

uint32_t set_pkg_sn(dp_req_t head);
void set_pkg_set_sn(dp_req_t head, uint32_t sn);

set_pkg_pack_state_t set_pkg_pack_state(dp_req_t head);
void set_pkg_set_pack_state(dp_req_t head, set_pkg_pack_state_t c);

set_pkg_category_t set_pkg_category(dp_req_t head);
void set_pkg_set_category(dp_req_t head, set_pkg_category_t c);

uint16_t set_pkg_from_svr_type(dp_req_t head);
uint16_t set_pkg_from_svr_id(dp_req_t head);
void set_pkg_set_from_svr(dp_req_t head, uint16_t from_svr_type, uint16_t from_svr_id);

uint16_t set_pkg_to_svr_type(dp_req_t head);
uint16_t set_pkg_to_svr_id(dp_req_t head);
void set_pkg_set_to_svr(dp_req_t head, uint16_t to_svr_type, uint16_t to_svr_id);

int set_pkg_carry_copy(dp_req_t carry, uint8_t size, void * data);
int set_pkg_carry_set_buf(dp_req_t carry, void * buf, size_t capacity);
uint8_t set_pkg_carry_capacity(dp_req_t carry);
int set_pkg_carry_set_size(dp_req_t carry, uint8_t size);
uint8_t set_pkg_carry_size(dp_req_t carry);
void * set_pkg_carry_data(dp_req_t carry);

#ifdef __cplusplus
}
#endif

#endif
