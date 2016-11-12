#ifndef USF_LOGIC_USE_DATA_BUF_H
#define USF_LOGIC_USE_DATA_BUF_H
#include "usf/logic/logic_types.h"
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

LPDRMETA logic_data_buf_meta(void);
logic_data_t logic_data_buf_find(logic_require_t r);
logic_data_t logic_data_buf_get_or_create(logic_require_t r, size_t capacity);

int logic_data_buf_capacity(logic_data_t data);
int logic_data_buf_size(logic_data_t data);
void * logic_data_buf(logic_data_t data);

#ifdef __cplusplus
}
#endif

#endif
