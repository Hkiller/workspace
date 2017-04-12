#ifndef SVR_CENTER_TYPES_H
#define SVR_CENTER_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum center_agent_pkg_category {
    center_agent_pkg_unknown = 0
    , center_agent_pkg_request = 1
    , center_agent_pkg_response = 2
    , center_agent_pkg_notify = 3
} center_agent_pkg_category_t;

typedef enum center_agent_pkg_pack_state {
    center_agent_pkg_not_packed = 0
    , center_agent_pkg_packed = 1
} center_agent_pkg_pack_state_t;

typedef struct center_agent * center_agent_t;
typedef struct center_agent_svr_type * center_agent_svr_type_t;
typedef struct center_agent_svr_stub * center_agent_svr_stub_t;

#ifdef __cplusplus
}
#endif

#endif
