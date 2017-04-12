#ifndef SVR_DBLOG_SVR_OPS_H
#define SVR_DBLOG_SVR_OPS_H
#include "version_svr.h"
#include "svr/center/agent/center_agent_types.h" 

/*protocol process ops*/
typedef void (*version_svr_op_t)(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void version_svr_op_query(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);

#endif
