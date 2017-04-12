#ifndef SVR_SET_SVR_CMD_INFO_H
#define SVR_SET_SVR_CMD_INFO_H
#include "set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_svr_cmd_info_t set_svr_cmd_info_find_by_name(set_svr_stub_t stub, set_svr_svr_info_t svr_info, const char * meta_name);

#ifdef __cplusplus
}
#endif

#endif
