#ifndef CPE_OTM_MEMO_H
#define CPE_OTM_MEMO_H
#include "otm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

otm_memo_t otm_memo_find(otm_timer_id_t id, otm_memo_t memo_buf, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
