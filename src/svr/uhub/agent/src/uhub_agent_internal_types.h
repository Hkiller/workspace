#ifndef SVR_UHUB_AGENT_INTERNAL_TYPES_H
#define SVR_UHUB_AGENT_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "svr/uhub/agent/uhub_agent_types.h"

struct uhub_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    set_svr_stub_t m_stub;
    uint16_t m_uhub_svr_type;
};

#endif
