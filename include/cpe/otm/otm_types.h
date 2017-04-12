#ifndef CPE_OTM_TYPES_H
#define CPE_OTM_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/tl/tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t otm_timer_id_t;

typedef struct otm_memo {
    otm_timer_id_t m_id;
    uint32_t m_last_action_time_s;
    uint32_t m_next_action_time_s;
} * otm_memo_t;

typedef struct otm_timer * otm_timer_t;
typedef struct otm_manage * otm_manage_t;

typedef void (*otm_process_fun_t) (otm_timer_t timer, otm_memo_t memo, uint32_t cur_exec_time_s, void * obj_ctx);

typedef struct otm_timer_it {
    otm_timer_t (*next)(struct otm_timer_it * it);
    char m_data[16];
} * otm_timer_it_t;

#ifdef __cplusplus
}
#endif

#endif
