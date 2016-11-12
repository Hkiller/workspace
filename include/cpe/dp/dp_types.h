#ifndef CPE_DP_TYPES_H
#define CPE_DP_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dp_rsp * dp_rsp_t;
typedef struct dp_rsp_it * dp_rsp_it_t;
typedef struct dp_req * dp_req_t;
typedef struct dp_mgr * dp_mgr_t;
typedef struct dp_rsp_type * dp_rsp_type_t;
typedef struct dp_binding * dp_binding_t;

typedef int (*dp_rsp_process_fun_t)(dp_req_t req, void * ctx, error_monitor_t em);

struct dp_rsp_type {
    const char * name;
    void (*destruct)(dp_rsp_t rsp);
};

typedef dp_rsp_t (*dp_rsp_it_next_fun)(dp_rsp_it_t it);

struct dp_rsp_it {
    dp_rsp_it_next_fun m_next_fun;
    void * m_context;
};

struct dp_binding_it {
    dp_binding_t (*next)(struct dp_binding_it * it);
    void * m_context;
};

typedef int (*dp_str_cmd_cvt_t)(int32_t * r, const char * str, void * ctx, error_monitor_t em);

typedef void (*dp_req_dump_fun_t)(dp_req_t req, write_stream_t s);

typedef struct dp_req_it {
    dp_req_t (*next)(struct dp_req_it * it);
    void * m_context;
} * dp_req_it_t;

#ifdef __cplusplus
}
#endif

#endif


