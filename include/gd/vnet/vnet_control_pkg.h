#ifndef GD_VNET_CONTROL_PKG_H
#define GD_VNET_CONTROL_PKG_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dp/dp_types.h"
#include "gd/app/app_types.h"
#include "vnet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * req_type_vnet_control_pkg;

vnet_control_pkg_t
vnet_control_pkg_create(gd_app_context_t app, size_t capacity);
void vnet_control_pkg_free(vnet_control_pkg_t pkg);

dp_req_t vnet_control_pkg_to_dp_req(vnet_control_pkg_t pkg);
vnet_control_pkg_t vnet_control_pkg_from_dp_req(dp_req_t pkg);

void vnet_control_pkg_init(vnet_control_pkg_t pkg);
uint32_t vnet_control_pkg_cmd(vnet_control_pkg_t pkg);
void vnet_control_pkg_set_cmd(vnet_control_pkg_t pkg, uint32_t cmd);

uint32_t vnet_control_pkg_connection_id(vnet_control_pkg_t pkg);
void vnet_control_pkg_set_connection_id(vnet_control_pkg_t pkg, uint32_t connection_id);

LPDRMETA vnet_control_pkg_data_meta(vnet_control_pkg_t pkg);
void * vnet_control_pkg_data_buf(vnet_control_pkg_t pkg);
size_t vnet_control_pkg_data_size(vnet_control_pkg_t pkg);
size_t vnet_control_pkg_data_capacity(vnet_control_pkg_t pkg);

int vnet_control_pkg_set_data(vnet_control_pkg_t pkg, LPDRMETA meta, void const * data, size_t size);
void * vnet_control_pkg_alloc_data(vnet_control_pkg_t pkg, LPDRMETA meta, size_t size);

#ifdef __cplusplus
}
#endif

#endif

