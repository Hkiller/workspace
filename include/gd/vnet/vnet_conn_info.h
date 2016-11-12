#ifndef GD_VNET_CONN_INFO_H
#define GD_VNET_CONN_INFO_H
#include "cpe/utils/hash_string.h"
#include "cpe/dp/dp_types.h"
#include "gd/app/app_types.h"
#include "vnet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_vnet_conn_info;

dp_req_t vnet_create_conn_info(gd_app_context_t app);
int64_t vnet_conn_info_conn_id(dp_req_t data);
void vnet_conn_info_set_conn_id(dp_req_t data, int64_t conn_id);

dp_req_t vnet_conn_info_check_or_create(dp_req_t target);
dp_req_t vnet_conn_info_find(dp_req_t target);

#ifdef __cplusplus
}
#endif

#endif

