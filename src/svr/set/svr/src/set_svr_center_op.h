#ifndef SVR_SET_SVR_TYPES_CENTER_OP_H
#define SVR_SET_SVR_TYPES_CENTER_OP_H
#include "set_svr_center.h"

void set_svr_center_send_sync_cmd(set_svr_center_t center);
void set_svr_center_on_sync_svrs(set_svr_center_t center, SVR_CENTER_RES_QUERY * syncing_res);
void set_svr_center_on_ntf_join(set_svr_center_t center, SVR_CENTER_NTF_JOIN * ntf_join);
void set_svr_center_on_ntf_leave(set_svr_center_t center, SVR_CENTER_NTF_LEAVE * ntf_leave);

#endif
