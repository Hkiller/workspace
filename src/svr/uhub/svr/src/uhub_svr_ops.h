#ifndef SVR_UHUB_SVR_OPS_H
#define SVR_UHUB_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "uhub_svr_types.h"
#include "svr/center/agent/center_agent_types.h" 

/*operations of uhub_svr */
uhub_svr_t
uhub_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em);

void uhub_svr_free(uhub_svr_t svr);

uhub_svr_t uhub_svr_find(gd_app_context_t app, cpe_hash_string_t name);
uhub_svr_t uhub_svr_find_nc(gd_app_context_t app, const char * name);
const char * uhub_svr_name(uhub_svr_t svr);

/*operations of uhub_svr_info */
uhub_svr_info_t uhub_svr_info_create(uhub_svr_t svr, const char * svr_type_name, const char * to_uid_entry);
void uhub_svr_info_free(uhub_svr_t svr, uhub_svr_info_t svr_info);

uhub_svr_info_t uhub_svr_info_find(uhub_svr_t svr, uint16_t svr_type);

/*operations of uhub_svr_notify_info */
uhub_svr_notify_info_t uhub_svr_notify_info_create(uhub_svr_t svr, uhub_svr_info_t svr_info, uint32_t cmd);
void uhub_svr_notify_info_free(uhub_svr_t svr, uhub_svr_notify_info_t notify_info);
void uhub_svr_notify_info_free_all(uhub_svr_t svr);

uhub_svr_notify_info_t uhub_svr_notify_info_find(uhub_svr_t svr, uint16_t svr_type, uint32_t cmd);

uint32_t uhub_svr_notify_info_hash(uhub_svr_notify_info_t);
int uhub_svr_notify_info_eq(uhub_svr_notify_info_t l, uhub_svr_notify_info_t r);

#endif
