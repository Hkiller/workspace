#ifndef SVR_SET_SVR_APP_H
#define SVR_SET_SVR_APP_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_svr_app_t set_svr_app_find_nc(gd_app_context_t app, const char * name);
set_svr_app_t set_svr_app_find(gd_app_context_t app, cpe_hash_string_t name);

gd_app_context_t set_svr_app_app(set_svr_app_t svr_app);
const char * set_svr_app_name(set_svr_app_t svr_app);
cpe_hash_string_t set_svr_app_name_hs(set_svr_app_t mgr);

#ifdef __cplusplus
}
#endif

#endif
