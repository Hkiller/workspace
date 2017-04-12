#ifndef SVR_DBLOG_AGENT_INTERNAL_TYPES_H
#define SVR_DBLOG_AGENT_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/pal/pal_socket.h"
#include "gd/timer/timer_types.h"
#include "svr/dblog/agent/dblog_agent.h"

struct dblog_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    uint16_t m_set_id;
    uint16_t m_set_type;
    char m_ip[16];
    uint16_t m_port;
    struct sockaddr_in m_target; 
    int m_fd;
    struct mem_buffer m_buffer;
};

dblog_agent_t
dblog_agent_create(
    gd_app_context_t app, const char * name,
    uint16_t set_id, uint16_t set_type,
    mem_allocrator_t alloc, error_monitor_t em);

void dblog_agent_free(dblog_agent_t svr);

int dblog_agent_set_target(dblog_agent_t svr, const char * ip, uint16_t port);

#endif
