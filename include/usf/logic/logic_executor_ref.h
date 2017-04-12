#ifndef USF_LOGIC_EXECUTOR_REF_H
#define USF_LOGIC_EXECUTOR_REF_H
#include "cpe/utils/stream.h"
#include "cpe/utils/hash_string.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_executor_ref_t
logic_executor_ref_find(logic_executor_mgr_t mgr, const char * name);

logic_executor_t logic_executor_ref_executor(logic_executor_ref_t ref);
const char * logic_executor_ref_name(logic_executor_ref_t ref);
uint16_t logic_executor_ref_count(logic_executor_ref_t ref);

void logic_executor_ref_inc(logic_executor_ref_t executor_ref);
void logic_executor_ref_dec(logic_executor_ref_t executor_ref);

#ifdef __cplusplus
}
#endif

#endif

