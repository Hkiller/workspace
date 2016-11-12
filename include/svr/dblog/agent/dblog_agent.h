#ifndef SVR_DBLOG_AGENT_H
#define SVR_DBLOG_AGENT_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/app/app_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "dblog_agent_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dblog_agent_t dblog_agent_find(gd_app_context_t app, cpe_hash_string_t name);
dblog_agent_t dblog_agent_find_nc(gd_app_context_t app, const char * name);
const char * dblog_agent_name(dblog_agent_t svr);

int dblog_agent_log(dblog_agent_t agent, void const * data, size_t data_size, LPDRMETA data_meta);

#ifdef __cplusplus
}
#endif

#endif

