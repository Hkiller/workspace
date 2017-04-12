#ifndef CPE_TL_ERRNO_H
#define CPE_TL_ERRNO_H
#include "tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum tl_error_no {
    CPE_TL_ERROR_NONE = 0
    , CPE_TL_ERROR_NO_MEMORY = -128
    , CPE_TL_ERROR_BAD_ARG
    , CPE_TL_ERROR_NO_TIME_SOURCE
    , CPE_TL_ERROR_EVENT_NO_DISPATCHER
    , CPE_TL_ERROR_EVENT_NO_ENQUEUE
    , CPE_TL_ERROR_EVENT_UNKNOWN
};

const char * tl_error_string(int e);

#ifdef __cplusplus
}
#endif

#endif
