#ifndef SVR_UHUB_AGENT_H
#define SVR_UHUB_AGENT_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "uhub_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uhub_agent_t uhub_agent_create(
    gd_app_context_t app,
    const char * name, set_svr_stub_t stub, uint16_t uhub_svr_type,
    mem_allocrator_t alloc, error_monitor_t em);

void uhub_agent_free(uhub_agent_t mgr);

uhub_agent_t uhub_agent_find(gd_app_context_t app, cpe_hash_string_t name);
uhub_agent_t uhub_agent_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t uhub_agent_app(uhub_agent_t mgr);
const char * uhub_agent_name(uhub_agent_t mgr);
cpe_hash_string_t uhub_agent_name_hs(uhub_agent_t mgr);

int uhub_agent_send_notify_pkg(
    uhub_agent_t agent, dp_req_t body,
    void const * carry_data, size_t carry_data_len);

int uhub_agent_send_notify_data(
    uhub_agent_t agent,
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_len);

#ifdef __cplusplus
}
#endif

#endif
