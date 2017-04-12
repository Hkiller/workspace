#ifndef SVR_CENTER_AGENT_SVR_TYPE_H
#define SVR_CENTER_AGENT_SVR_TYPE_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_types.h"
#include "center_agent_types.h"

#ifdef __cplusplus
extern "C" {
#endif

center_agent_svr_type_t center_agent_svr_type_create(center_agent_t agent, const char * svr_type_name);
void center_agent_svr_type_free(center_agent_svr_type_t group);

center_agent_svr_type_t center_agent_svr_type_check_create(center_agent_t agent, const char * svr_type_name);

center_agent_svr_type_t center_agent_svr_type_find(center_agent_t agent, uint16_t svr_type);
center_agent_svr_type_t center_agent_svr_type_lsearch_by_name(center_agent_t agent, const char * svr_type_name);

LPDRMETA center_agent_svr_type_pkg_meta(center_agent_svr_type_t type);
const char * center_agent_svr_type_name(center_agent_svr_type_t svr_type);

uint16_t center_agent_svr_type_id(center_agent_svr_type_t svr_type);

cfg_t center_agent_svr_type_cfg(center_agent_svr_type_t svr_type);

#ifdef __cplusplus
}
#endif

#endif
