#ifndef GD_NET_TRANS_TYPES_H
#define GD_NET_TRANS_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    net_trans_task_init
    , net_trans_task_working
    , net_trans_task_done
} net_trans_task_state_t;

typedef enum {
    net_trans_result_unknown
    , net_trans_result_ok
    , net_trans_result_error
    , net_trans_result_timeout
    , net_trans_result_cancel
} net_trans_task_result_t;

typedef enum {
    net_trans_errno_none = 0    
    , net_trans_errno_range_error = 33
} net_trans_errno_t;

typedef struct net_trans_manage * net_trans_manage_t;
typedef struct net_trans_group * net_trans_group_t;
typedef struct net_trans_task * net_trans_task_t;

typedef void (*net_trans_task_commit_op_t)(net_trans_task_t task, void * ctx);
typedef void (*net_trans_task_progress_op_t)(net_trans_task_t task, void * ctx, double dltotal, double dlnow);
typedef void (*net_trans_task_write_op_t)(net_trans_task_t task, void * ctx, void * data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif
