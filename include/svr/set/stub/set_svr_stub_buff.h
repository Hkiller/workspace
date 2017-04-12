#ifndef SVR_SET_SVR_BUFF_H
#define SVR_SET_SVR_BUFF_H
#include "set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_svr_stub_buff_t set_svr_stub_buff_check_create(set_svr_stub_t stub, const char * name, uint64_t capacity);
set_svr_stub_buff_t set_svr_stub_buff_find(set_svr_stub_t stub, const char * name);

uint8_t set_svr_stub_buff_is_init(set_svr_stub_buff_t buff);
void set_svr_stub_buff_set_init(set_svr_stub_buff_t buff, uint8_t is_init);

set_svr_stub_buff_type_t set_svr_stub_buff_type(set_svr_stub_buff_t buff);
uint64_t set_svr_stub_buff_capacity(set_svr_stub_buff_t buff);
void * set_svr_stub_buff_data(set_svr_stub_buff_t buff);

#ifdef __cplusplus
}
#endif

#endif
