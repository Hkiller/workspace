#ifndef SVR_SET_STUB_TYPES_H
#define SVR_SET_STUB_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/app/app_types.h"
#include "svr/set/share/set_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct set_svr_stub * set_svr_stub_t;
typedef struct set_svr_app * set_svr_app_t;
typedef struct set_svr_svr_info * set_svr_svr_info_t;
typedef struct set_svr_cmd_info * set_svr_cmd_info_t;
typedef struct set_svr_stub_buff * set_svr_stub_buff_t;

typedef enum set_svr_stub_buff_type {
    set_svr_stub_buff_type_mem = 1
    , set_svr_stub_buff_type_shm = 2
} set_svr_stub_buff_type_t;

#ifdef __cplusplus
}
#endif

#endif
