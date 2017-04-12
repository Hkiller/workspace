#ifndef SVR_DBLOG_SVR_OPS_H
#define SVR_DBLOG_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "dir_svr.h"
#include "svr/center/agent/center_agent_types.h" 

/*protocol process ops*/
typedef void (*dir_svr_op_t)(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void dir_svr_op_query_regions(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);
void dir_svr_op_query_servers(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body);

#endif
