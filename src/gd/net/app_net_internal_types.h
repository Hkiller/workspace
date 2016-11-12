#ifndef GD_NET_INTERNAL_TYPES_H
#define GD_NET_INTERNAL_TYPES_H
#include "gd/app/app_types.h"

struct app_net_runner {
    gd_app_context_t m_app;
    int64_t m_tick_span;
};

#endif
