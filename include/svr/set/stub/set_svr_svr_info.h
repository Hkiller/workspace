#ifndef SVR_SET_SVR_SVR_INFO_H
#define SVR_SET_SVR_SVR_INFO_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_svr_svr_info_t
set_svr_svr_info_find(set_svr_stub_t svr, uint16_t svr_type);
set_svr_svr_info_t
set_svr_svr_info_find_by_name(set_svr_stub_t svr, const char * svr_name);
set_svr_svr_info_t
set_svr_svr_info_find_by_meta(set_svr_stub_t svr, const char * meta_name);

uint16_t set_svr_svr_info_svr_type_id(set_svr_svr_info_t svr_info);
const char * set_svr_svr_info_svr_type_name(set_svr_svr_info_t svr_info);
LPDRMETA set_svr_svr_info_pkg_meta(set_svr_svr_info_t svr_info);
LPDRMETAENTRY set_svr_svr_info_pkg_data_entry(set_svr_svr_info_t svr_info);
LPDRMETAENTRY set_svr_svr_info_pkg_cmd_entry(set_svr_svr_info_t svr_info);
LPDRMETA set_svr_svr_info_carry_meta(set_svr_svr_info_t svr_info);
LPDRMETA set_svr_svr_info_find_data_meta_by_cmd(set_svr_svr_info_t svr_info, uint32_t cmd);

LPDRMETA set_svr_svr_info_error_pkg_meta(set_svr_svr_info_t svr_info);
uint32_t set_svr_svr_info_error_pkg_cmd(set_svr_svr_info_t svr_info);
LPDRMETAENTRY set_svr_svr_info_error_pkg_errno_entry(set_svr_svr_info_t svr_info);

#ifdef __cplusplus
}
#endif

#endif
