#ifndef SVR_CENTER_AGENT_INTERNAL_OPS_H
#define SVR_CENTER_AGENT_INTERNAL_OPS_H
#include "center_agent_internal_types.h"
#include "protocol/svr/center/svr_center_pro.h"

/*center_agent_svr_type*/
void center_agent_svr_type_free_all(center_agent_t agent);

uint32_t center_agent_svr_type_hash(center_agent_svr_type_t group);
int center_agent_svr_type_eq(center_agent_svr_type_t l, center_agent_svr_type_t r);

#endif
